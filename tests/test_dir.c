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
#include <libzenit/io.h>
#include <libzenit/result.h>
#include "test_runner.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TMP_DIR "libzenit_test_dir"
#define TMP_DIR_NESTED "libzenit_test_dir/sub/a/b"
#define TMP_DIR_EMPTY "libzenit_test_dir_empty"
#define TMP_DIR_LIST "libzenit_test_dir_list"
#define NONEXISTENT "libzenit_test_dir_nonexistent"

static void cleanup(void) {
    /* Remove test files first, then directories bottom-up */
    zenit_file_delete(TMP_DIR_LIST "/file_a.txt");
    zenit_file_delete(TMP_DIR_LIST "/file_b.txt");
    zenit_file_delete(TMP_DIR "/file.txt");
    zenit_dir_remove(TMP_DIR_LIST);
    zenit_dir_remove(TMP_DIR_NESTED);
    zenit_dir_remove(TMP_DIR_EMPTY);
    zenit_dir_remove(TMP_DIR "/sub/a/b");
    zenit_dir_remove(TMP_DIR "/sub/a");
    zenit_dir_remove(TMP_DIR "/sub");
    zenit_dir_remove(TMP_DIR);
    (void)zenit_dir_remove(NONEXISTENT);
}

static int test_create_and_exists(void) {
    zenit_result_t r = zenit_dir_create(TMP_DIR);
    if (r.error != ZENIT_OK) {
        FAIL("create failed");
        return 1;
    }
    if (!zenit_dir_exists(TMP_DIR)) {
        FAIL("exists after create returned 0");
        return 1;
    }
    PASS();
    return 0;
}

static int test_create_nested(void) {
    zenit_result_t r = zenit_dir_create(TMP_DIR_NESTED);
    if (r.error != ZENIT_OK) {
        FAIL("create nested failed");
        return 1;
    }
    if (!zenit_dir_exists(TMP_DIR_NESTED)) {
        FAIL("nested dir does not exist");
        return 1;
    }
    PASS();
    return 0;
}

static int test_create_exists_already(void) {
    /* Creating an existing directory should succeed */
    zenit_result_t r = zenit_dir_create(TMP_DIR);
    if (r.error != ZENIT_OK) {
        FAIL("create existing should succeed");
        return 1;
    }
    PASS();
    return 0;
}

static int test_list_empty(void) {
    zenit_result_t r = zenit_dir_create(TMP_DIR_EMPTY);
    if (r.error != ZENIT_OK) {
        FAIL("create empty dir failed");
        return 1;
    }

    char **names = NULL;
    size_t count = 0;
    r = zenit_dir_list(TMP_DIR_EMPTY, &names, &count);
    if (r.error != ZENIT_OK) {
        FAIL("list empty dir failed");
        return 1;
    }
    /* Free the array and strings (even an empty list should be cleanly freed) */
    for (size_t i = 0; i < count; i++) {
        free(names[i]);
    }
    free(names);
    PASS();
    return 0;
}

static int test_list_with_entries(void) {
    zenit_result_t r = zenit_dir_create(TMP_DIR_LIST);
    if (r.error != ZENIT_OK) {
        FAIL("create list dir failed");
        return 1;
    }

    /* Create a couple of files inside */
    zenit_file_write(TMP_DIR_LIST "/file_a.txt", "a", 1);
    zenit_file_write(TMP_DIR_LIST "/file_b.txt", "b", 1);

    char **names = NULL;
    size_t count = 0;
    r = zenit_dir_list(TMP_DIR_LIST, &names, &count);
    if (r.error != ZENIT_OK) {
        FAIL("list with entries failed");
        return 1;
    }

    /* We expect at least 2 entries (the two files, plus possibly . and ..) */
    int found_a = 0;
    int found_b = 0;
    for (size_t i = 0; i < count; i++) {
        if (strcmp(names[i], "file_a.txt") == 0) found_a = 1;
        if (strcmp(names[i], "file_b.txt") == 0) found_b = 1;
        free(names[i]);
    }
    free(names);

    if (!found_a) {
        FAIL("file_a.txt not found in listing");
        return 1;
    }
    if (!found_b) {
        FAIL("file_b.txt not found in listing");
        return 1;
    }
    PASS();
    return 0;
}

static int test_iterate(void) {
    int found_a = 0;

    zenit_dir_iter_t *iter = zenit_dir_iter(TMP_DIR_LIST);
    if (iter == NULL) {
        FAIL("iter open failed");
        return 1;
    }

    zenit_dir_entry_t entry;
    while (zenit_dir_next(iter, &entry)) {
        if (strcmp(entry.name, "file_a.txt") == 0) {
            found_a = 1;
        }
    }
    zenit_dir_iter_destroy(iter);

    if (!found_a) {
        FAIL("iter did not find file_a.txt");
        return 1;
    }
    PASS();
    return 0;
}

static int test_remove(void) {
    /* Ensure the directory exists first */
    zenit_dir_create(TMP_DIR_EMPTY);

    zenit_result_t r = zenit_dir_remove(TMP_DIR_EMPTY);
    if (r.error != ZENIT_OK) {
        FAIL("remove failed");
        return 1;
    }
    if (zenit_dir_exists(TMP_DIR_EMPTY)) {
        FAIL("dir still exists after remove");
        return 1;
    }
    PASS();
    return 0;
}

static int test_nonexistent(void) {
    /* List on non-existent dir should return NOT_FOUND */
    char **names = NULL;
    size_t count = 0;
    zenit_result_t r = zenit_dir_list(NONEXISTENT, &names, &count);
    if (r.error != ZENIT_ERROR_NOT_FOUND) {
        FAIL("list nonexistent should return ZENIT_ERROR_NOT_FOUND");
        return 1;
    }

    /* Iter on non-existent dir should return NULL */
    zenit_dir_iter_t *iter = zenit_dir_iter(NONEXISTENT);
    if (iter != NULL) {
        FAIL("iter on nonexistent should return NULL");
        zenit_dir_iter_destroy(iter);
        return 1;
    }

    /* Remove on non-existent dir should return NOT_FOUND */
    r = zenit_dir_remove(NONEXISTENT);
    if (r.error != ZENIT_ERROR_NOT_FOUND) {
        FAIL("remove nonexistent should return ZENIT_ERROR_NOT_FOUND");
        return 1;
    }

    /* Exists on non-existent should return 0 */
    if (zenit_dir_exists(NONEXISTENT)) {
        FAIL("exists nonexistent should return 0");
        return 1;
    }

    PASS();
    return 0;
}

static int test_null_params(void) {
    if (zenit_dir_create(NULL).error != ZENIT_ERROR_NULL) {
        FAIL("create NULL should return ZENIT_ERROR_NULL");
        return 1;
    }

    if (zenit_dir_remove(NULL).error != ZENIT_ERROR_NULL) {
        FAIL("remove NULL should return ZENIT_ERROR_NULL");
        return 1;
    }

    if (zenit_dir_exists(NULL) != 0) {
        FAIL("exists NULL should return 0");
        return 1;
    }

    if (zenit_dir_iter(NULL) != NULL) {
        FAIL("iter NULL should return NULL");
        return 1;
    }

    {
        zenit_dir_entry_t entry;
        if (zenit_dir_next(NULL, &entry) != 0) {
            FAIL("next with NULL iter should return 0");
            return 1;
        }
    }

    {
        /* Build a valid iter first */
        zenit_dir_iter_t *iter = zenit_dir_iter(TMP_DIR_LIST);
        if (iter != NULL) {
            if (zenit_dir_next(iter, NULL) != 0) {
                FAIL("next with NULL out_entry should return 0");
                zenit_dir_iter_destroy(iter);
                return 1;
            }
            zenit_dir_iter_destroy(iter);
        }
    }

    /* NULL path for list */
    {
        char **names = NULL;
        size_t count = 0;
        if (zenit_dir_list(NULL, &names, &count).error != ZENIT_ERROR_NULL) {
            FAIL("list NULL path should return ZENIT_ERROR_NULL");
            return 1;
        }
    }

    /* NULL out_names for list */
    {
        size_t count = 0;
        if (zenit_dir_list(TMP_DIR_LIST, NULL, &count).error != ZENIT_ERROR_NULL) {
            FAIL("list NULL out_names should return ZENIT_ERROR_NULL");
            return 1;
        }
    }

    /* NULL out_count for list */
    {
        char **names = NULL;
        if (zenit_dir_list(TMP_DIR_LIST, &names, NULL).error != ZENIT_ERROR_NULL) {
            FAIL("list NULL out_count should return ZENIT_ERROR_NULL");
            return 1;
        }
    }

    /* iter_destroy NULL is a no-op — just verify it doesn't crash */
    zenit_dir_iter_destroy(NULL);

    PASS();
    return 0;
}

static int test_create_too_long(void) {
    char long_path[1030];
    memset(long_path, 'a', 1025);
    long_path[1025] = '\0';
    zenit_result_t r = zenit_dir_create(long_path);
    ASSERT(r.error == ZENIT_ERROR_PARAM, "too-long path should return PARAM error");
    PASS();
    return 0;
}

static int test_null_exists(void) {
    ASSERT(zenit_dir_exists(NULL) == 0, "exists NULL should return 0");
    PASS();
    return 0;
}

static int test_null_remove(void) {
    ASSERT(zenit_dir_remove(NULL).error == ZENIT_ERROR_NULL, "remove NULL should return NULL error");
    PASS();
    return 0;
}

static int test_list_null_param(void) {
    char **names = NULL;
    size_t count = 0;
    ASSERT(zenit_dir_list(NULL, &names, &count).error == ZENIT_ERROR_NULL, "list NULL path");
    ASSERT(zenit_dir_list(TMP_DIR_LIST, NULL, &count).error == ZENIT_ERROR_NULL, "list NULL out_names");
    ASSERT(zenit_dir_list(TMP_DIR_LIST, &names, NULL).error == ZENIT_ERROR_NULL, "list NULL out_count");
    PASS();
    return 0;
}

static int test_list_second_pass(void) {
    char **names = NULL;
    size_t count = 0;
    zenit_result_t r = zenit_dir_list(TMP_DIR_LIST, &names, &count);
    ASSERT(r.error == ZENIT_OK, "list second pass ok");
    for (size_t i = 0; i < count; i++) free(names[i]);
    free(names);
    PASS();
    return 0;
}

static int test_null_next(void) {
    zenit_dir_entry_t entry;
    ASSERT(zenit_dir_next(NULL, &entry) == 0, "next NULL iter");
    ASSERT(zenit_dir_next(NULL, NULL) == 0, "next NULL all");
    PASS();
    return 0;
}

static int test_null_iter(void) {
    ASSERT(zenit_dir_iter(NULL) == NULL, "iter NULL path");
    PASS();
    return 0;
}

static int test_create_trailing_slashes(void) {
    zenit_result_t r = zenit_dir_create(TMP_DIR "/sub///");
    ASSERT(r.error == ZENIT_OK, "create with trailing slashes");
    ASSERT(zenit_dir_exists(TMP_DIR "/sub"), "dir with trailing slashes exists");
    zenit_dir_remove(TMP_DIR "/sub");
    PASS();
    return 0;
}

int main(void) {
    cleanup();

    int (*tests[])(void) = {
        test_create_and_exists,
        test_create_nested,
        test_create_exists_already,
        test_list_empty,
        test_list_with_entries,
        test_iterate,
        test_remove,
        test_nonexistent,
        test_null_params,
        test_create_too_long,
        test_null_exists,
        test_null_remove,
        test_list_null_param,
        test_list_second_pass,
        test_null_next,
        test_null_iter,
        test_create_trailing_slashes,
    };
    const char *names[] = {
        "create and exists",
        "create nested",
        "create exists already",
        "list empty",
        "list with entries",
        "iterate",
        "remove",
        "nonexistent",
        "NULL params",
        "create too long",
        "NULL exists",
        "NULL remove",
        "list NULL param",
        "list second pass",
        "NULL next",
        "NULL iter",
        "create trailing slashes",
    };
    ZENIT_RUN_TESTS("dir", tests, names);

    cleanup();
    return 0;
}
