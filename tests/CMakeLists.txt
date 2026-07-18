#
#    LibZenit
#    Copyright (C) 2026  Ian Torres
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Affero General Public License version 3
#    as published by the Free Software Foundation.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Affero General Public License for more details.
#
#    You should have received a copy of the GNU Affero General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

function(zenit_add_test name source)
  add_executable(${name} ${source})
  target_link_libraries(${name} PRIVATE zenit)

  if(LIBZENIT_BUILD_COVERAGE)
    target_compile_options(${name} PRIVATE --coverage -g -O0)
    target_link_options(${name} PRIVATE --coverage)
  endif()

  add_test(NAME ${name} COMMAND ${name})
endfunction()

function(zenit_add_malloc_fail_test name source)
  if(NOT (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND NOT APPLE)))
    return()
  endif()

  add_executable(${name} ${source})
  target_link_libraries(${name} PRIVATE zenit)
  target_link_options(${name} PRIVATE -Wl,--wrap=malloc -Wl,--wrap=calloc)

  if(LIBZENIT_BUILD_COVERAGE)
    target_compile_options(${name} PRIVATE --coverage -g -O0)
    target_link_options(${name} PRIVATE --coverage)
  endif()

  add_test(NAME ${name} COMMAND ${name})
endfunction()

# ─── Regular tests ───
zenit_add_test(test_version     test_version.c)
zenit_add_test(test_result      test_result.c)
zenit_add_test(test_ring        test_ring.c)
zenit_add_test(test_state       test_state.c)
zenit_add_test(test_benchmark   test_benchmark.c)
zenit_add_test(test_arena       test_arena.c)
zenit_add_test(test_vector      test_vector.c)
zenit_add_test(test_map         test_map.c)
zenit_add_test(test_set         test_set.c)
zenit_add_test(test_string      test_string.c)
zenit_add_test(test_list        test_list.c)
zenit_add_test(test_heap        test_heap.c)
zenit_add_test(test_deque       test_deque.c)
zenit_add_test(test_bitset      test_bitset.c)
zenit_add_test(test_json        test_json.c)
zenit_add_test(test_json_cov    test_json_cov.c)

# ─── Stack & queue tests ───
zenit_add_test(test_stack       test_stack.c)
zenit_add_test(test_queue       test_queue.c)

# ─── Timer test (no alloc, no malloc-fail) ───
zenit_add_test(test_timer       test_timer.c)

# ─── Pool test ───
zenit_add_test(test_pool        test_pool.c)

# ─── I/O tests ───
zenit_add_test(test_io          test_io.c)

# ─── Allocator test ───
zenit_add_test(test_allocator   test_allocator.c)

# ─── Encoding / string / sort tests ───
zenit_add_test(test_base64      test_base64.c)
zenit_add_test(test_hex         test_hex.c)
zenit_add_test(test_uri         test_uri.c)
zenit_add_test(test_str         test_str.c)
zenit_add_test(test_sort        test_sort.c)

# ─── Malloc-fail tests (--wrap=malloc,calloc) ───
zenit_add_malloc_fail_test(test_ring_malloc_fail   test_ring_malloc_fail.c)
zenit_add_malloc_fail_test(test_state_malloc_fail  test_state_malloc_fail.c)
zenit_add_malloc_fail_test(test_arena_malloc_fail  test_arena_malloc_fail.c)
zenit_add_malloc_fail_test(test_map_malloc_fail    test_map_malloc_fail.c)
zenit_add_malloc_fail_test(test_set_malloc_fail    test_set_malloc_fail.c)
zenit_add_malloc_fail_test(test_list_malloc_fail   test_list_malloc_fail.c)
zenit_add_malloc_fail_test(test_stack_malloc_fail  test_stack_malloc_fail.c)
zenit_add_malloc_fail_test(test_queue_malloc_fail  test_queue_malloc_fail.c)
zenit_add_malloc_fail_test(test_pool_malloc_fail   test_pool_malloc_fail.c)
zenit_add_malloc_fail_test(test_io_malloc_fail     test_io_malloc_fail.c)
zenit_add_malloc_fail_test(test_hex_malloc_fail    test_hex_malloc_fail.c)
zenit_add_malloc_fail_test(test_uri_malloc_fail    test_uri_malloc_fail.c)

# Malloc-fail tests that also need --wrap=realloc
function(zenit_add_malloc_fail_realloc_test name source)
  if(NOT (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND NOT APPLE)))
    return()
  endif()

  add_executable(${name} ${source})
  target_link_libraries(${name} PRIVATE zenit)
  target_link_options(${name} PRIVATE -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc)

  if(LIBZENIT_BUILD_COVERAGE)
    target_compile_options(${name} PRIVATE --coverage -g -O0)
    target_link_options(${name} PRIVATE --coverage)
  endif()

  add_test(NAME ${name} COMMAND ${name})
endfunction()

zenit_add_malloc_fail_realloc_test(test_bitset_malloc_fail  test_bitset_malloc_fail.c)
zenit_add_malloc_fail_realloc_test(test_vector_malloc_fail  test_vector_malloc_fail.c)
zenit_add_malloc_fail_realloc_test(test_heap_malloc_fail    test_heap_malloc_fail.c)
zenit_add_malloc_fail_realloc_test(test_deque_malloc_fail   test_deque_malloc_fail.c)
zenit_add_malloc_fail_realloc_test(test_string_malloc_fail  test_string_malloc_fail.c)
zenit_add_malloc_fail_realloc_test(test_json_malloc_fail    test_json_malloc_fail.c)
zenit_add_malloc_fail_test(test_base64_malloc_fail test_base64_malloc_fail.c)
zenit_add_malloc_fail_realloc_test(test_str_malloc_fail test_str_malloc_fail.c)
