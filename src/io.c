//
//    LibZenit
//    Copyright (C) 2026  Ian Torres
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Affero General Public License version 3
//    as published by the Free Software Foundation.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public License
//    along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include <libzenit/io.h>
#include <libzenit/allocator.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <Windows.h>
#else
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#define IO_COPY_BUF_SIZE 65536

zenit_result_t zenit_file_read_with_allocator(const char *path, void **out_data, size_t *out_len, zenit_allocator_t allocator) {
    if (path == NULL || out_data == NULL || out_len == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

#if defined(_WIN32)
    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }

    LARGE_INTEGER li;
    if (!GetFileSizeEx(h, &li)) {
        CloseHandle(h);
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }
    size_t size = (size_t)li.QuadPart;

    void *buf = allocator.alloc_fn(size + 1, allocator.ctx);
    if (buf == NULL) {
        CloseHandle(h);
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    DWORD bytes_read;
    if (!ReadFile(h, buf, (DWORD)size, &bytes_read, NULL)) {
        allocator.free_fn(buf, allocator.ctx);
        CloseHandle(h);
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }
    CloseHandle(h);
    ((unsigned char *)buf)[bytes_read] = '\0';

    *out_data = buf;
    *out_len = (size_t)bytes_read;
    return ZENIT_RESULT_OK;
#else
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        close(fd);
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }
    size_t size = (size_t)st.st_size;

    void *buf = allocator.alloc_fn(size + 1, allocator.ctx);
    if (buf == NULL) {
        close(fd);
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }

    ssize_t bytes_read = read(fd, buf, size);
    if (bytes_read < 0 || (size_t)bytes_read != size) {
        allocator.free_fn(buf, allocator.ctx);
        close(fd);
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }
    close(fd);
    ((unsigned char *)buf)[bytes_read] = '\0';

    *out_data = buf;
    *out_len = (size_t)bytes_read;
    return ZENIT_RESULT_OK;
#endif
}

zenit_result_t zenit_file_read(const char *path, void **out_data, size_t *out_len) {
    return zenit_file_read_with_allocator(path, out_data, out_len, ZENIT_ALLOCATOR_DEFAULT);
}

static zenit_result_t file_write_impl(const char *path, const void *data, size_t len, int append) {
    if (path == NULL || data == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

#if defined(_WIN32)
    DWORD flags = append ? OPEN_ALWAYS : CREATE_ALWAYS;
    HANDLE h = CreateFileA(path, GENERIC_WRITE, 0, NULL, flags, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }
    if (append) {
        SetFilePointer(h, 0, NULL, FILE_END);
    }
    DWORD bytes_written;
    if (!WriteFile(h, data, (DWORD)len, &bytes_written, NULL)) {
        CloseHandle(h);
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }
    CloseHandle(h);
    if ((size_t)bytes_written != len) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_SIZE);
    }
#else
    int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int fd = open(path, flags, mode);
    if (fd < 0) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }
    ssize_t bytes_written = write(fd, data, len);
    if (bytes_written < 0 || (size_t)bytes_written != len) {
        close(fd);
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_SIZE);
    }
    close(fd);
#endif
    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_file_write(const char *path, const void *data, size_t len) {
    return file_write_impl(path, data, len, 0);
}

zenit_result_t zenit_file_append(const char *path, const void *data, size_t len) {
    return file_write_impl(path, data, len, 1);
}

int zenit_file_exists(const char *path) {
    if (path == NULL) return 0;
#if defined(_WIN32)
    return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES ? 1 : 0;
#else
    return access(path, F_OK) == 0 ? 1 : 0;
#endif
}

zenit_result_t zenit_file_delete(const char *path) {
    if (path == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
#if defined(_WIN32)
    if (!DeleteFileA(path)) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }
#else
    if (unlink(path) < 0) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }
#endif
    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_file_size(const char *path, size_t *out_size) {
    if (path == NULL || out_size == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }
#if defined(_WIN32)
    WIN32_FILE_ATTRIBUTE_DATA info;
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &info)) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }
    LARGE_INTEGER li;
    li.LowPart = info.nFileSizeLow;
    li.HighPart = info.nFileSizeHigh;
    *out_size = (size_t)li.QuadPart;
#else
    struct stat st;
    if (stat(path, &st) < 0) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }
    *out_size = (size_t)st.st_size;
#endif
    return ZENIT_RESULT_OK;
}

zenit_result_t zenit_file_copy(const char *src, const char *dst) {
    if (src == NULL || dst == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

#if defined(_WIN32)
    if (!CopyFileA(src, dst, FALSE)) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }
    return ZENIT_RESULT_OK;
#else
    int fd_src = open(src, O_RDONLY);
    if (fd_src < 0) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }

    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int fd_dst = open(dst, O_WRONLY | O_CREAT | O_TRUNC, mode);
    if (fd_dst < 0) {
        close(fd_src);
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }

    unsigned char buf[IO_COPY_BUF_SIZE];
    ssize_t n;
    while ((n = read(fd_src, buf, sizeof(buf))) > 0) {
        ssize_t written = write(fd_dst, buf, (size_t)n);
        if (written != n) {
            close(fd_src);
            close(fd_dst);
            return ZENIT_RESULT_ERROR(ZENIT_ERROR_SIZE);
        }
    }

    close(fd_src);
    close(fd_dst);

    if (n < 0) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }
    return ZENIT_RESULT_OK;
#endif
}
