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

# ---------------------------------------------------------------------------
# Benchmarks — built only when LIBZENIT_BUILD_BENCHMARKS=ON
#
# Each benchmark is a standalone C executable linked against libzenit.
# They register with CTest under the "benchmark" label so they can be
# filtered:  ctest -L benchmark   (run only benchmarks)
#            ctest -LE benchmark  (run everything except benchmarks)
#
# A custom "benchmark" target is also provided for convenience:
#   cmake --build build --target benchmark
# ---------------------------------------------------------------------------

# Version call overhead
add_executable(benchmark_version benchmark_version.c)
target_link_libraries(benchmark_version PRIVATE zenit)
add_test(NAME benchmark_version COMMAND benchmark_version)
set_tests_properties(benchmark_version PROPERTIES LABELS "benchmark")

# State-machine throughput
add_executable(benchmark_state benchmark_state.c)
target_link_libraries(benchmark_state PRIVATE zenit)
add_test(NAME benchmark_state COMMAND benchmark_state)
set_tests_properties(benchmark_state PROPERTIES LABELS "benchmark")

# Arena allocator throughput
add_executable(benchmark_arena benchmark_arena.c)
target_link_libraries(benchmark_arena PRIVATE zenit)
add_test(NAME benchmark_arena COMMAND benchmark_arena)
set_tests_properties(benchmark_arena PROPERTIES LABELS "benchmark")

# Ring-buffer throughput
add_executable(benchmark_ring benchmark_ring.c)
target_link_libraries(benchmark_ring PRIVATE zenit)
add_test(NAME benchmark_ring COMMAND benchmark_ring)
set_tests_properties(benchmark_ring PROPERTIES LABELS "benchmark")

# Vector throughput
add_executable(benchmark_vector benchmark_vector.c)
target_link_libraries(benchmark_vector PRIVATE zenit)
add_test(NAME benchmark_vector COMMAND benchmark_vector)
set_tests_properties(benchmark_vector PROPERTIES LABELS "benchmark")

# Hash map throughput
add_executable(benchmark_map benchmark_map.c)
target_link_libraries(benchmark_map PRIVATE zenit)
add_test(NAME benchmark_map COMMAND benchmark_map)
set_tests_properties(benchmark_map PROPERTIES LABELS "benchmark")

# Convenience target — runs all benchmarks sequentially
add_custom_target(benchmark
  COMMAND ${CMAKE_COMMAND} -E echo ""
  COMMAND ${CMAKE_COMMAND} -E echo "===== LibZenit Benchmarks ====="
  COMMAND ${CMAKE_COMMAND} -E echo ""
  COMMAND "$<TARGET_FILE:benchmark_version>"
  COMMAND "$<TARGET_FILE:benchmark_state>"
  COMMAND "$<TARGET_FILE:benchmark_arena>"
  COMMAND "$<TARGET_FILE:benchmark_ring>"
  COMMAND "$<TARGET_FILE:benchmark_vector>"
  COMMAND "$<TARGET_FILE:benchmark_map>"
  COMMAND ${CMAKE_COMMAND} -E echo ""
  COMMAND ${CMAKE_COMMAND} -E echo "===== Done ====="
  USES_TERMINAL
)
