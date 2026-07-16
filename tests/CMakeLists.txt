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

add_executable(test_state
  test_state.c
)

target_link_libraries(test_state PRIVATE zenit)

if(LIBZENIT_BUILD_COVERAGE)
  target_compile_options(test_state PRIVATE --coverage -g -O0)
  target_link_options(test_state PRIVATE --coverage)
endif()

add_test(NAME test_state COMMAND test_state)

add_executable(test_malloc_fail
  test_malloc_fail.c
)

target_link_libraries(test_malloc_fail PRIVATE zenit)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  target_link_options(test_malloc_fail PRIVATE -Wl,--wrap=malloc)
endif()

add_test(NAME test_malloc_fail COMMAND test_malloc_fail)

add_test(NAME test_libzenit COMMAND test_libzenit)
