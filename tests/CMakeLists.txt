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
zenit_add_test(test_libzenit    test_version.c)
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

# ─── Malloc-fail tests (--wrap) ───
zenit_add_malloc_fail_test(test_ring_malloc_fail   test_ring_malloc_fail.c)
zenit_add_malloc_fail_test(test_state_malloc_fail  test_state_malloc_fail.c)
zenit_add_malloc_fail_test(test_arena_malloc_fail  test_arena_malloc_fail.c)
zenit_add_malloc_fail_test(test_map_malloc_fail    test_map_malloc_fail.c)
zenit_add_malloc_fail_test(test_set_malloc_fail    test_set_malloc_fail.c)
zenit_add_malloc_fail_test(test_list_malloc_fail   test_list_malloc_fail.c)

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
