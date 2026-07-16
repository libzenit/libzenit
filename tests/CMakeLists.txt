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
  target_link_options(test_state_malloc_fail PRIVATE -Wl,--wrap=malloc)

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

add_test(NAME test_libzenit COMMAND test_libzenit)
