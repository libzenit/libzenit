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

#include <libzenit/glob.h>
#include <stdio.h>

#define ASSERT_MATCH(p, s, expected) do { \
    int got = zenit_glob_match((p), (s)); \
    if (got != (expected)) { \
        fprintf(stderr, "FAIL: glob '%s' vs '%s' expected %d got %d\n", \
                (const char*)(p), (const char*)(s), (expected), got); \
        return 1; \
    } \
} while(0)

int main(void) {
    /* Test 1: Exact match */
    ASSERT_MATCH("hello", "hello", 1);
    printf("PASS: glob exact\n");

    /* Test 2: Star wildcard */
    ASSERT_MATCH("*.txt", "readme.txt", 1);
    ASSERT_MATCH("*.txt", "readme.md", 0);
    printf("PASS: glob star\n");

    /* Test 3: Question mark */
    ASSERT_MATCH("hello.?", "hello.c", 1);
    ASSERT_MATCH("hello.?", "hello.cm", 0);
    printf("PASS: glob question\n");

    /* Test 4: Star matches empty */
    ASSERT_MATCH("test*", "test", 1);
    ASSERT_MATCH("*test", "test", 1);
    ASSERT_MATCH("*", "", 1);
    ASSERT_MATCH("*", "anything", 1);
    printf("PASS: glob star empty\n");

    /* Test 5: Multiple stars */
    ASSERT_MATCH("a*b*c", "abc", 1);
    ASSERT_MATCH("a*b*c", "aXbYc", 1);
    ASSERT_MATCH("a*b*c", "aXbYcZ", 0);
    printf("PASS: glob multi star\n");

    /* Test 6: Character class */
    ASSERT_MATCH("[abc]", "a", 1);
    ASSERT_MATCH("[abc]", "b", 1);
    ASSERT_MATCH("[abc]", "d", 0);
    ASSERT_MATCH("file_[abc].txt", "file_b.txt", 1);
    ASSERT_MATCH("file_[abc].txt", "file_d.txt", 0);
    printf("PASS: glob char class\n");

    /* Test 7: Character range */
    ASSERT_MATCH("[a-z]", "m", 1);
    ASSERT_MATCH("[a-z]", "M", 0);
    ASSERT_MATCH("[0-9]", "5", 1);
    printf("PASS: glob char range\n");

    /* Test 8: Negated character class */
    ASSERT_MATCH("[!abc]", "d", 1);
    ASSERT_MATCH("[!abc]", "a", 0);
    printf("PASS: glob negated class\n");

    /* Test 9: NULL params */
    ASSERT_MATCH(NULL, "test", 0);
    ASSERT_MATCH("test", NULL, 0);
    ASSERT_MATCH(NULL, NULL, 0);
    printf("PASS: glob NULL params\n");

    /* Test 10: Complex patterns */
    ASSERT_MATCH("src/**/*.c", "src/**/*.c", 1);  /* ** is not special, treated as literal */
    ASSERT_MATCH("?at", "cat", 1);
    ASSERT_MATCH("?at", "at", 0);
    ASSERT_MATCH("*at", "at", 1);
    ASSERT_MATCH("*at", "chat", 1);
    printf("PASS: glob complex\n");

    printf("PASS: glob\n");
    return 0;
}
