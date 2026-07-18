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

cmake_minimum_required(VERSION 3.20)
project(libzenit VERSION 0.1.0 LANGUAGES C)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(LIBZENIT_BUILD_TESTS "Build tests" ON)
option(LIBZENIT_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(LIBZENIT_BUILD_COVERAGE "Build with coverage instrumentation" OFF)

enable_testing()

add_subdirectory(src)

if(LIBZENIT_BUILD_TESTS)
  add_subdirectory(tests)
endif()

if(LIBZENIT_BUILD_BENCHMARKS)
  add_subdirectory(benchmarks)
endif()

include(GNUInstallDirs)

install(
  DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  TARGETS zenit
  EXPORT libzenit-targets
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  EXPORT libzenit-targets
  FILE libzenitTargets.cmake
  NAMESPACE libzenit::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libzenit
)

find_program(DOXYGEN_EXECUTABLE doxygen)
if(DOXYGEN_EXECUTABLE)
  add_custom_target(docs
    COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_SOURCE_DIR}/Doxyfile
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen"
    USES_TERMINAL
  )
endif()
