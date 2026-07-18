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

#include <libzenit/dir.h>
#include <libzenit/result.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <Windows.h>   /* Windows API — header name matches platform casing */
#include <direct.h>
#else
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#endif

/* ─── Internal struct definitions ─── */

#if defined(_WIN32)
struct zenit_dir_iter_t {
    HANDLE handle;
    WIN32_FIND_DATAA find_data;
    int first;         /* 1 if find_data still holds the first result */
};
#else
struct zenit_dir_iter_t {
    DIR *dir;
};
#endif

/* ─── mkdir -p helper ─── */

/*
 * Walk the path component by component, creating each directory
 * as we go.  Skips components that already exist.
 * Uses dynamically allocated buffer to support paths of any length.
 */
static zenit_result_t create_dir_recursive(const char *path) {
    size_t len = strlen(path);
    char *tmp = malloc(len + 1);
    if (tmp == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
    }
    memcpy(tmp, path, len + 1);

    /* Strip trailing slashes so we don't create empty components */
    while (len > 0 && (tmp[len - 1] == '/' || tmp[len - 1] == '\\')) {
        tmp[--len] = '\0';
    }

    /* Guard: after stripping, the path may be empty */
    if (len == 0) {
        free(tmp);
        return ZENIT_RESULT_OK;
    }

    /* Walk each '/' or '\' separator */
    for (char *p = tmp + 1; *p != '\0'; p++) {
        if (*p == '/' || *p == '\\') {
            /* Temporarily terminate the string at this separator */
            char saved = *p;
            *p = '\0';

#if defined(_WIN32)
            if (_mkdir(tmp) != 0 && errno != EEXIST) {
                *p = saved;
                free(tmp);
                return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
            }
#else
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                *p = saved;
                free(tmp);
                return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
            }
#endif

            /* Restore the separator and advance */
            *p = saved;
        }
    }

    /* Create the final (deepest) component */
#if defined(_WIN32)
    if (_mkdir(tmp) != 0 && errno != EEXIST) {
        free(tmp);
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }
#else
    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        free(tmp);
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }
#endif

    free(tmp);
    return ZENIT_RESULT_OK;
}

/* ─── Public API ─── */

zenit_result_t zenit_dir_create(const char *path) {
    if (path == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    /* Short-cut: if the path already exists as a directory, succeed */
    if (zenit_dir_exists(path)) {
        return ZENIT_RESULT_OK;
    }

    return create_dir_recursive(path);
}

zenit_result_t zenit_dir_remove(const char *path) {
    if (path == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

#if defined(_WIN32)
    if (!RemoveDirectoryA(path)) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }
#else
    if (rmdir(path) != 0) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }
#endif

    return ZENIT_RESULT_OK;
}

int zenit_dir_exists(const char *path) {
    if (path == NULL) return 0;

#if defined(_WIN32)
    /* Use GetFileAttributes to check both existence and directory flag */
    DWORD attrs = GetFileAttributesA(path);
    if (attrs == INVALID_FILE_ATTRIBUTES) return 0;
    return (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0 ? 1 : 0;
#else
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISDIR(st.st_mode) ? 1 : 0;
#endif
}

zenit_dir_iter_t* zenit_dir_iter(const char *path) {
    if (path == NULL) return NULL;

#if defined(_WIN32)
    /* Build search pattern: path\* */
    size_t path_len = strlen(path);
    char *search = malloc(path_len + 3);
    if (search == NULL) return NULL;

    memcpy(search, path, path_len);

    /* Remove trailing slashes before appending \* */
    while (path_len > 0 && (search[path_len - 1] == '/' || search[path_len - 1] == '\\')) {
        search[--path_len] = '\0';
    }

    search[path_len] = '\\';
    search[path_len + 1] = '*';
    search[path_len + 2] = '\0';

    WIN32_FIND_DATAA find_data;
    HANDLE h = FindFirstFileA(search, &find_data);
    free(search);
    if (h == INVALID_HANDLE_VALUE) return NULL;

    /* Allocate the iterator handle */
    zenit_dir_iter_t *iter = malloc(sizeof(zenit_dir_iter_t));
    if (iter == NULL) {
        FindClose(h);
        return NULL;
    }

    iter->handle = h;
    iter->find_data = find_data;
    iter->first = 1;
    return iter;
#else
    DIR *dir = opendir(path);
    if (dir == NULL) return NULL;

    zenit_dir_iter_t *iter = malloc(sizeof(zenit_dir_iter_t));
    if (iter == NULL) {
        closedir(dir);
        return NULL;
    }

    iter->dir = dir;
    return iter;
#endif
}

int zenit_dir_next(zenit_dir_iter_t *iter, zenit_dir_entry_t *out_entry) {
    if (iter == NULL || out_entry == NULL) return 0;

#if defined(_WIN32)
    /* First call — use the data cached by FindFirstFile */
    if (iter->first) {
        iter->first = 0;
    } else {
        /* Subsequent calls — advance with FindNextFile */
        if (!FindNextFileA(iter->handle, &iter->find_data)) {
            return 0;
        }
    }

    /* Copy the filename, truncating if necessary */
    size_t name_len = strlen(iter->find_data.cFileName);
    if (name_len >= sizeof(out_entry->name)) {
        name_len = sizeof(out_entry->name) - 1;
    }
    memcpy(out_entry->name, iter->find_data.cFileName, name_len);
    out_entry->name[name_len] = '\0';

    /* Check the directory flag */
    out_entry->is_directory =
        (iter->find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;

    return 1;
#else
    struct dirent *entry = readdir(iter->dir);
    if (entry == NULL) return 0;

    /* Copy the filename, truncating if necessary */
    size_t name_len = strlen(entry->d_name);
    if (name_len >= sizeof(out_entry->name)) {
        name_len = sizeof(out_entry->name) - 1;
    }
    memcpy(out_entry->name, entry->d_name, name_len);
    out_entry->name[name_len] = '\0';

    /* d_type is available on modern POSIX systems (Linux, macOS, BSD) */
    out_entry->is_directory = (entry->d_type == DT_DIR) ? 1 : 0;

    return 1;
#endif
}

void zenit_dir_iter_destroy(zenit_dir_iter_t *iter) {
    if (iter == NULL) return;

#if defined(_WIN32)
    FindClose(iter->handle);
#else
    closedir(iter->dir);
#endif

    free(iter);
}

zenit_result_t zenit_dir_list(const char *path, char ***out_names, size_t *out_count) {
    if (path == NULL || out_names == NULL || out_count == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL);
    }

    zenit_dir_entry_t entry;

    /* First pass: open the directory and count entries */
    zenit_dir_iter_t *iter = zenit_dir_iter(path);
    if (iter == NULL) {
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }

    size_t count = 0;
    while (zenit_dir_next(iter, &entry)) {
        count++;
    }
    zenit_dir_iter_destroy(iter);

    /* Allocate the pointer array for all entry names */
    char **names = NULL;
    if (count > 0) {
        names = calloc(count, sizeof(char *));
        if (names == NULL) {
            return ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
        }
    }

    /* Second pass: re-open and populate the array.
     * Use i as the actual write index, bounded by count from the first pass.
     * If the directory changed between passes (more entries now), we stop at count. */
    iter = zenit_dir_iter(path);
    if (iter == NULL) {
        free(names);
        return ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND);
    }

    size_t i = 0;
    zenit_result_t result = ZENIT_RESULT_OK;
    while (zenit_dir_next(iter, &entry) && i < count) {
        names[i] = malloc(strlen(entry.name) + 1);
        if (names[i] == NULL) {
            result = ZENIT_RESULT_ERROR(ZENIT_ERROR_ALLOC);
            break;
        }
        size_t slen = strlen(entry.name);
        char *dst = names[i];
        for (size_t c = 0; c <= slen; c++) {
            dst[c] = entry.name[c];
        }
        i++;
    }

    zenit_dir_iter_destroy(iter);

    if (result.error != ZENIT_OK) {
        for (size_t j = 0; j < i; j++) {
            free(names[j]);
        }
        free(names);
        return result;
    }

    *out_names = names;
    *out_count = i;
    return ZENIT_RESULT_OK;
}
