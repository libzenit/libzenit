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

add_executable(test_libzenit
  test_version.c
)

target_link_libraries(test_libzenit PRIVATE zenit)

if(LIBZENIT_BUILD_COVERAGE)
  target_compile_options(test_libzenit PRIVATE --coverage -g -O0)
  target_link_options(test_libzenit PRIVATE --coverage)
endif()

add_executable(test_result
  test_result.c
)

target_link_libraries(test_result PRIVATE zenit)

if(LIBZENIT_BUILD_COVERAGE)
  target_compile_options(test_result PRIVATE --coverage -g -O0)
  target_link_options(test_result PRIVATE --coverage)
endif()

add_test(NAME test_result COMMAND test_result)

add_executable(test_ring
  test_ring.c
)

target_link_libraries(test_ring PRIVATE zenit)

if(LIBZENIT_BUILD_COVERAGE)
  target_compile_options(test_ring PRIVATE --coverage -g -O0)
  target_link_options(test_ring PRIVATE --coverage)
endif()

add_test(NAME test_ring COMMAND test_ring)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND NOT APPLE))
  add_executable(test_ring_malloc_fail
    test_ring_malloc_fail.c
  )

  target_link_libraries(test_ring_malloc_fail PRIVATE zenit)
  target_link_options(test_ring_malloc_fail PRIVATE -Wl,--wrap=malloc -Wl,--wrap=calloc)

  if(LIBZENIT_BUILD_COVERAGE)
    target_compile_options(test_ring_malloc_fail PRIVATE --coverage -g -O0)
    target_link_options(test_ring_malloc_fail PRIVATE --coverage)
  endif()

  add_test(NAME test_ring_malloc_fail COMMAND test_ring_malloc_fail)
endif()

add_executable(test_state
  test_state.c
)

target_link_libraries(test_state PRIVATE zenit)

if(LIBZENIT_BUILD_COVERAGE)
  target_compile_options(test_state PRIVATE --coverage -g -O0)
  target_link_options(test_state PRIVATE --coverage)
endif()

add_test(NAME test_state COMMAND test_state)

add_executable(test_benchmark
  test_benchmark.c
)

target_link_libraries(test_benchmark PRIVATE zenit)

if(LIBZENIT_BUILD_COVERAGE)
  target_compile_options(test_benchmark PRIVATE --coverage -g -O0)
  target_link_options(test_benchmark PRIVATE --coverage)
endif()

add_test(NAME test_benchmark COMMAND test_benchmark)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND NOT APPLE))
  add_executable(test_state_malloc_fail
    test_state_malloc_fail.c
  )

  target_link_libraries(test_state_malloc_fail PRIVATE zenit)
  target_link_options(test_state_malloc_fail PRIVATE -Wl,--wrap=malloc -Wl,--wrap=calloc)

  if(LIBZENIT_BUILD_COVERAGE)
    target_compile_options(test_state_malloc_fail PRIVATE --coverage -g -O0)
    target_link_options(test_state_malloc_fail PRIVATE --coverage)
  endif()

  add_test(NAME test_state_malloc_fail COMMAND test_state_malloc_fail)
endif()

add_executable(test_arena
  test_arena.c
)

target_link_libraries(test_arena PRIVATE zenit)

if(LIBZENIT_BUILD_COVERAGE)
  target_compile_options(test_arena PRIVATE --coverage -g -O0)
  target_link_options(test_arena PRIVATE --coverage)
endif()

add_test(NAME test_arena COMMAND test_arena)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND NOT APPLE))
  add_executable(test_arena_malloc_fail
    test_arena_malloc_fail.c
  )

  target_link_libraries(test_arena_malloc_fail PRIVATE zenit)
  target_link_options(test_arena_malloc_fail PRIVATE -Wl,--wrap=malloc -Wl,--wrap=calloc)

  if(LIBZENIT_BUILD_COVERAGE)
    target_compile_options(test_arena_malloc_fail PRIVATE --coverage -g -O0)
    target_link_options(test_arena_malloc_fail PRIVATE --coverage)
  endif()

  add_test(NAME test_arena_malloc_fail COMMAND test_arena_malloc_fail)
endif()

add_executable(test_vector
  test_vector.c
)

target_link_libraries(test_vector PRIVATE zenit)

if(LIBZENIT_BUILD_COVERAGE)
  target_compile_options(test_vector PRIVATE --coverage -g -O0)
  target_link_options(test_vector PRIVATE --coverage)
endif()

add_test(NAME test_vector COMMAND test_vector)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND NOT APPLE))
  add_executable(test_vector_malloc_fail
    test_vector_malloc_fail.c
  )

  target_link_libraries(test_vector_malloc_fail PRIVATE zenit)
  target_link_options(test_vector_malloc_fail PRIVATE -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc)

  if(LIBZENIT_BUILD_COVERAGE)
    target_compile_options(test_vector_malloc_fail PRIVATE --coverage -g -O0)
    target_link_options(test_vector_malloc_fail PRIVATE --coverage)
  endif()

  add_test(NAME test_vector_malloc_fail COMMAND test_vector_malloc_fail)
endif()

add_executable(test_map
  test_map.c
)

target_link_libraries(test_map PRIVATE zenit)

if(LIBZENIT_BUILD_COVERAGE)
  target_compile_options(test_map PRIVATE --coverage -g -O0)
  target_link_options(test_map PRIVATE --coverage)
endif()

add_test(NAME test_map COMMAND test_map)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND NOT APPLE))
  add_executable(test_map_malloc_fail
    test_map_malloc_fail.c
  )

  target_link_libraries(test_map_malloc_fail PRIVATE zenit)
  target_link_options(test_map_malloc_fail PRIVATE -Wl,--wrap=malloc -Wl,--wrap=calloc)

  if(LIBZENIT_BUILD_COVERAGE)
    target_compile_options(test_map_malloc_fail PRIVATE --coverage -g -O0)
    target_link_options(test_map_malloc_fail PRIVATE --coverage)
  endif()

  add_test(NAME test_map_malloc_fail COMMAND test_map_malloc_fail)
endif()

add_executable(test_set
  test_set.c
)

target_link_libraries(test_set PRIVATE zenit)

if(LIBZENIT_BUILD_COVERAGE)
  target_compile_options(test_set PRIVATE --coverage -g -O0)
  target_link_options(test_set PRIVATE --coverage)
endif()

add_test(NAME test_set COMMAND test_set)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND NOT APPLE))
  add_executable(test_set_malloc_fail
    test_set_malloc_fail.c
  )

  target_link_libraries(test_set_malloc_fail PRIVATE zenit)
  target_link_options(test_set_malloc_fail PRIVATE -Wl,--wrap=malloc -Wl,--wrap=calloc)

  if(LIBZENIT_BUILD_COVERAGE)
    target_compile_options(test_set_malloc_fail PRIVATE --coverage -g -O0)
    target_link_options(test_set_malloc_fail PRIVATE --coverage)
  endif()

  add_test(NAME test_set_malloc_fail COMMAND test_set_malloc_fail)
endif()

add_test(NAME test_libzenit COMMAND test_libzenit)

add_executable(test_list
  test_list.c
)

target_link_libraries(test_list PRIVATE zenit)

if(LIBZENIT_BUILD_COVERAGE)
  target_compile_options(test_list PRIVATE --coverage -g -O0)
  target_link_options(test_list PRIVATE --coverage)
endif()

add_test(NAME test_list COMMAND test_list)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND NOT APPLE))
  add_executable(test_list_malloc_fail
    test_list_malloc_fail.c
  )

  target_link_libraries(test_list_malloc_fail PRIVATE zenit)
  target_link_options(test_list_malloc_fail PRIVATE -Wl,--wrap=malloc -Wl,--wrap=calloc)

  if(LIBZENIT_BUILD_COVERAGE)
    target_compile_options(test_list_malloc_fail PRIVATE --coverage -g -O0)
    target_link_options(test_list_malloc_fail PRIVATE --coverage)
  endif()

  add_test(NAME test_list_malloc_fail COMMAND test_list_malloc_fail)
endif()

add_executable(test_heap
  test_heap.c
)

target_link_libraries(test_heap PRIVATE zenit)

if(LIBZENIT_BUILD_COVERAGE)
  target_compile_options(test_heap PRIVATE --coverage -g -O0)
  target_link_options(test_heap PRIVATE --coverage)
endif()

add_test(NAME test_heap COMMAND test_heap)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND NOT APPLE))
  add_executable(test_heap_malloc_fail
    test_heap_malloc_fail.c
  )

  target_link_libraries(test_heap_malloc_fail PRIVATE zenit)
  target_link_options(test_heap_malloc_fail PRIVATE -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc)

  if(LIBZENIT_BUILD_COVERAGE)
    target_compile_options(test_heap_malloc_fail PRIVATE --coverage -g -O0)
    target_link_options(test_heap_malloc_fail PRIVATE --coverage)
  endif()

  add_test(NAME test_heap_malloc_fail COMMAND test_heap_malloc_fail)
endif()

add_executable(test_deque
  test_deque.c
)

target_link_libraries(test_deque PRIVATE zenit)

if(LIBZENIT_BUILD_COVERAGE)
  target_compile_options(test_deque PRIVATE --coverage -g -O0)
  target_link_options(test_deque PRIVATE --coverage)
endif()

add_test(NAME test_deque COMMAND test_deque)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND NOT APPLE))
  add_executable(test_deque_malloc_fail
    test_deque_malloc_fail.c
  )

  target_link_libraries(test_deque_malloc_fail PRIVATE zenit)
  target_link_options(test_deque_malloc_fail PRIVATE -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc)

  if(LIBZENIT_BUILD_COVERAGE)
    target_compile_options(test_deque_malloc_fail PRIVATE --coverage -g -O0)
    target_link_options(test_deque_malloc_fail PRIVATE --coverage)
  endif()

  add_test(NAME test_deque_malloc_fail COMMAND test_deque_malloc_fail)
endif()
