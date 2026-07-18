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

# ─── Directory tests ───
zenit_add_test(test_dir         test_dir.c)

# ─── Bloom filter tests ───
zenit_add_test(test_bloom      test_bloom.c)

# ─── Trie tests ───
zenit_add_test(test_trie       test_trie.c)

# ─── LRU cache tests ───
zenit_add_test(test_lru        test_lru.c)

# ─── Graph tests ───
zenit_add_test(test_graph      test_graph.c)

# ─── Allocator test ───
zenit_add_test(test_allocator   test_allocator.c)

# ─── Encoding / string / sort tests ───
zenit_add_test(test_base64      test_base64.c)
zenit_add_test(test_hex         test_hex.c)
zenit_add_test(test_uri         test_uri.c)
zenit_add_test(test_str         test_str.c)
zenit_add_test(test_path        test_path.c)
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
zenit_add_test(test_io_malloc_fail     test_io_malloc_fail.c)
zenit_add_malloc_fail_test(test_lru_malloc_fail    test_lru_malloc_fail.c)
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
zenit_add_malloc_fail_realloc_test(test_str_malloc_fail test_str_malloc_fail.c)
zenit_add_malloc_fail_realloc_test(test_allocator_malloc_fail test_allocator_malloc_fail.c)
zenit_add_malloc_fail_test(test_path_malloc_fail test_path_malloc_fail.c)
zenit_add_malloc_fail_test(test_dir_malloc_fail   test_dir_malloc_fail.c)
zenit_add_malloc_fail_test(test_base64_malloc_fail test_base64_malloc_fail.c)
zenit_add_malloc_fail_test(test_bloom_malloc_fail test_bloom_malloc_fail.c)
zenit_add_malloc_fail_test(test_trie_malloc_fail test_trie_malloc_fail.c)
zenit_add_malloc_fail_test(test_graph_malloc_fail test_graph_malloc_fail.c)
zenit_add_malloc_fail_test(test_sort_malloc_fail test_sort_malloc_fail.c)

# ─── New module tests ───
zenit_add_test(test_uuid             test_uuid.c)
zenit_add_test(test_semver           test_semver.c)
zenit_add_test(test_logger           test_logger.c)
zenit_add_test(test_glob             test_glob.c)
zenit_add_test(test_spsc             test_spsc.c)
# Header-only, no link needed
add_executable(test_optional test_optional.c)
target_link_libraries(test_optional PRIVATE zenit)
add_test(NAME test_optional COMMAND test_optional)
zenit_add_test(test_csv              test_csv.c)
zenit_add_test(test_thread_pool      test_thread_pool.c)

zenit_add_malloc_fail_test(test_semver_malloc_fail        test_semver_malloc_fail.c)
zenit_add_malloc_fail_test(test_logger_malloc_fail        test_logger_malloc_fail.c)
zenit_add_malloc_fail_test(test_spsc_malloc_fail          test_spsc_malloc_fail.c)
zenit_add_malloc_fail_test(test_csv_malloc_fail           test_csv_malloc_fail.c)
zenit_add_malloc_fail_test(test_thread_pool_malloc_fail   test_thread_pool_malloc_fail.c)
# No-op malloc-fail tests (modules that don't allocate internally)
zenit_add_test(test_uuid_malloc_fail          test_uuid_malloc_fail.c)
zenit_add_test(test_glob_malloc_fail          test_glob_malloc_fail.c)
