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

add_library(zenit STATIC
  allocator.c
  result.c
  version.c
  state.c
  arena.c
  benchmark.c
  ring.c
  vector.c
  map.c
  set.c
  list.c
  heap.c
  deque.c
  string.c
  bitset.c
)

target_include_directories(zenit
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  target_compile_options(zenit PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(LIBZENIT_BUILD_COVERAGE)
  target_compile_options(zenit PRIVATE --coverage -g -O0)
  target_link_options(zenit PRIVATE --coverage)
endif()
