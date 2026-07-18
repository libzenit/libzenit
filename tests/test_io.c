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
#include <libzenit/result.h>
#include "test_runner.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Use relative paths — works in the test working directory on all platforms */
#define TMPFILE "libzenit_test_io.tmp"
#define TMPFILE2 "libzenit_test_io_copy.tmp"
#define TMPFILE_APPEND "libzenit_test_io_append.tmp"
#define TMPFILE_DEL "libzenit_test_io_del.tmp"
#define NONEXISTENT "libzenit_test_io_nonexistent.tmp"
#define NONEXISTENT_SIZE "libzenit_test_io_nonexistent_size.tmp"
#define NONEXISTENT_COPY "libzenit_test_io_nonexistent_copy.tmp"

static void cleanup(void) {
    (void)zenit_file_delete(TMPFILE);
    (void)zenit_file_delete(TMPFILE2);
    (void)zenit_file_delete(TMPFILE_APPEND);
    (void)zenit_file_delete(TMPFILE_DEL);
    (void)zenit_file_delete(NONEXISTENT_COPY);
}

static int test_write_and_read(void) {
    const char *content = "Hello, LibZenit!";
    size_t content_len = strlen(content);

    zenit_result_t r = zenit_file_write(TMPFILE, content, content_len);
    if (r.error != ZENIT_OK) {
        FAIL("write failed");
        return 1;
    }

    void *buf = NULL;
    size_t len = 0;
    r = zenit_file_read(TMPFILE, &buf, &len);
    if (r.error != ZENIT_OK) {
        FAIL("read failed");
        return 1;
    }
    if (len != content_len) {
        FAIL("read length mismatch");
        free(buf);
        return 1;
    }
    if (memcmp(buf, content, content_len) != 0) {
        FAIL("read content mismatch");
        free(buf);
        return 1;
    }
    free(buf);
    PASS();
    return 0;
}

static int test_read_with_allocator(void) {
    const char *content = "allocator test";
    size_t content_len = strlen(content);

    zenit_file_write(TMPFILE, content, content_len);

    void *buf = NULL;
    size_t len = 0;
    zenit_result_t r = zenit_file_read_with_allocator(TMPFILE, &buf, &len, ZENIT_ALLOCATOR_DEFAULT);
    if (r.error != ZENIT_OK) {
        FAIL("read_with_allocator failed");
        return 1;
    }
    if (len != content_len || memcmp(buf, content, content_len) != 0) {
        FAIL("read_with_allocator content mismatch");
        free(buf);
        return 1;
    }
    free(buf);
    PASS();
    return 0;
}

static int test_append(void) {
    const char *part1 = "Hello, ";
    const char *part2 = "World!";
    size_t expected_len = strlen(part1) + strlen(part2);

    zenit_result_t r = zenit_file_write(TMPFILE_APPEND, part1, strlen(part1));
    if (r.error != ZENIT_OK) {
        FAIL("append write part1 failed");
        return 1;
    }

    r = zenit_file_append(TMPFILE_APPEND, part2, strlen(part2));
    if (r.error != ZENIT_OK) {
        FAIL("append failed");
        return 1;
    }

    void *buf = NULL;
    size_t len = 0;
    r = zenit_file_read(TMPFILE_APPEND, &buf, &len);
    if (r.error != ZENIT_OK) {
        FAIL("append read failed");
        return 1;
    }
    if (len != expected_len) {
        FAIL("append length mismatch");
        free(buf);
        return 1;
    }
    if (memcmp(buf, "Hello, World!", expected_len) != 0) {
        FAIL("append content mismatch");
        free(buf);
        return 1;
    }
    free(buf);
    PASS();
    return 0;
}

static int test_exists(void) {
    if (!zenit_file_exists(TMPFILE)) {
        FAIL("exists on real file returned 0");
        return 1;
    }
    if (zenit_file_exists(NONEXISTENT)) {
        FAIL("exists on nonexistent returned 1");
        return 1;
    }
    PASS();
    return 0;
}

static int test_delete(void) {
    zenit_file_write(TMPFILE_DEL, "delete me", 9);

    zenit_result_t r = zenit_file_delete(TMPFILE_DEL);
    if (r.error != ZENIT_OK) {
        FAIL("delete failed");
        return 1;
    }
    if (zenit_file_exists(TMPFILE_DEL)) {
        FAIL("file still exists after delete");
        return 1;
    }
    PASS();
    return 0;
}

static int test_size(void) {
    const char *content = "1234567890";
    size_t expected = strlen(content);
    zenit_file_write(TMPFILE, content, expected);

    size_t sz = 0;
    zenit_result_t r = zenit_file_size(TMPFILE, &sz);
    if (r.error != ZENIT_OK) {
        FAIL("size failed");
        return 1;
    }
    if (sz != expected) {
        FAIL("size mismatch");
        return 1;
    }
    PASS();
    return 0;
}

static int test_copy(void) {
    zenit_file_delete(TMPFILE2);
    const char *content = "Copy me!";
    size_t content_len = strlen(content);
    zenit_file_write(TMPFILE, content, content_len);

    zenit_result_t r = zenit_file_copy(TMPFILE, TMPFILE2);
    if (r.error != ZENIT_OK) {
        FAIL("copy failed");
        return 1;
    }
    if (!zenit_file_exists(TMPFILE2)) {
        FAIL("copy destination does not exist");
        return 1;
    }

    size_t sz = 0;
    zenit_file_size(TMPFILE2, &sz);
    if (sz != content_len) {
        FAIL("copy size mismatch");
        return 1;
    }

    void *buf = NULL;
    size_t len = 0;
    r = zenit_file_read(TMPFILE2, &buf, &len);
    if (r.error != ZENIT_OK) {
        FAIL("copy read failed");
        return 1;
    }
    if (memcmp(buf, content, content_len) != 0) {
        FAIL("copy content mismatch");
        free(buf);
        return 1;
    }
    free(buf);
    PASS();
    return 0;
}

static int test_null_params(void) {
    void *buf = NULL;
    size_t len = 0;

    if (zenit_file_read(NULL, &buf, &len).error != ZENIT_ERROR_NULL) {
        FAIL("read NULL path should return ZENIT_ERROR_NULL");
        return 1;
    }
    if (zenit_file_read(TMPFILE, NULL, &len).error != ZENIT_ERROR_NULL) {
        FAIL("read NULL out_data should return ZENIT_ERROR_NULL");
        return 1;
    }
    if (zenit_file_read(TMPFILE, &buf, NULL).error != ZENIT_ERROR_NULL) {
        FAIL("read NULL out_len should return ZENIT_ERROR_NULL");
        return 1;
    }
    if (zenit_file_write(NULL, "x", 1).error != ZENIT_ERROR_NULL) {
        FAIL("write NULL path should return ZENIT_ERROR_NULL");
        return 1;
    }
    if (zenit_file_write(TMPFILE, NULL, 1).error != ZENIT_ERROR_NULL) {
        FAIL("write NULL data should return ZENIT_ERROR_NULL");
        return 1;
    }
    if (zenit_file_append(NULL, "x", 1).error != ZENIT_ERROR_NULL) {
        FAIL("append NULL path should return ZENIT_ERROR_NULL");
        return 1;
    }
    if (zenit_file_append(TMPFILE, NULL, 1).error != ZENIT_ERROR_NULL) {
        FAIL("append NULL data should return ZENIT_ERROR_NULL");
        return 1;
    }
    if (zenit_file_delete(NULL).error != ZENIT_ERROR_NULL) {
        FAIL("delete NULL path should return ZENIT_ERROR_NULL");
        return 1;
    }
    {
        size_t sz;
        if (zenit_file_size(NULL, &sz).error != ZENIT_ERROR_NULL) {
            FAIL("size NULL path should return ZENIT_ERROR_NULL");
            return 1;
        }
        if (zenit_file_size(TMPFILE, NULL).error != ZENIT_ERROR_NULL) {
            FAIL("size NULL out_size should return ZENIT_ERROR_NULL");
            return 1;
        }
    }
    if (zenit_file_copy(NULL, TMPFILE).error != ZENIT_ERROR_NULL) {
        FAIL("copy NULL src should return ZENIT_ERROR_NULL");
        return 1;
    }
    if (zenit_file_copy(TMPFILE, NULL).error != ZENIT_ERROR_NULL) {
        FAIL("copy NULL dst should return ZENIT_ERROR_NULL");
        return 1;
    }
    PASS();
    return 0;
}

static int test_read_nonexistent(void) {
    void *buf = NULL;
    size_t len = 0;
    zenit_result_t r = zenit_file_read(NONEXISTENT, &buf, &len);
    if (r.error != ZENIT_ERROR_NOT_FOUND) {
        FAIL("read nonexistent should return ZENIT_ERROR_NOT_FOUND");
        return 1;
    }
    PASS();
    return 0;
}

static int test_size_nonexistent(void) {
    size_t sz;
    zenit_result_t r = zenit_file_size(NONEXISTENT_SIZE, &sz);
    if (r.error != ZENIT_ERROR_NOT_FOUND) {
        FAIL("size nonexistent should return ZENIT_ERROR_NOT_FOUND");
        return 1;
    }
    PASS();
    return 0;
}

static int test_write_bad_path(void) {
    zenit_result_t r = zenit_file_write("/nonexistent_dir_libzenit_foo.tmp", "data", 4);
    if (r.error != ZENIT_ERROR_NOT_FOUND) {
        FAIL("write to bad path should return ZENIT_ERROR_NOT_FOUND");
        return 1;
    }
    PASS();
    return 0;
}

static int test_copy_nonexistent_src(void) {
    zenit_result_t r = zenit_file_copy(NONEXISTENT_COPY, "libzenit_test_io_dst.tmp");
    if (r.error != ZENIT_ERROR_NOT_FOUND) {
        FAIL("copy nonexistent src should return ZENIT_ERROR_NOT_FOUND");
        return 1;
    }
    (void)zenit_file_delete("libzenit_test_io_dst.tmp");
    PASS();
    return 0;
}

static int test_copy_bad_dst_dir(void) {
    zenit_file_write(TMPFILE, "content", 7);
    zenit_result_t r = zenit_file_copy(TMPFILE, "/nonexistent_dir_libzenit_copy.tmp");
    if (r.error != ZENIT_ERROR_NOT_FOUND) {
        FAIL("copy to bad dst dir should return ZENIT_ERROR_NOT_FOUND");
        return 1;
    }
    PASS();
    return 0;
}

int main(void) {
    cleanup();

    int (*tests[])(void) = {
        test_write_and_read,
        test_read_with_allocator,
        test_append,
        test_exists,
        test_delete,
        test_size,
        test_size_nonexistent,
        test_copy,
        test_copy_nonexistent_src,
        test_copy_bad_dst_dir,
        test_write_bad_path,
        test_null_params,
        test_read_nonexistent,
    };
    const char *names[] = {
        "write and read",
        "read with allocator",
        "append",
        "exists",
        "delete",
        "size",
        "size nonexistent",
        "copy",
        "copy nonexistent src",
        "copy bad dst dir",
        "write bad path",
        "NULL params",
        "read nonexistent",
    };
    ZENIT_RUN_TESTS("io", tests, names);

    cleanup();
    return 0;
}
