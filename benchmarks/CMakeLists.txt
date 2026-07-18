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

# Hash set throughput
add_executable(benchmark_set benchmark_set.c)
target_link_libraries(benchmark_set PRIVATE zenit)
add_test(NAME benchmark_set COMMAND benchmark_set)
set_tests_properties(benchmark_set PROPERTIES LABELS "benchmark")

# String throughput
add_executable(benchmark_string benchmark_string.c)
target_link_libraries(benchmark_string PRIVATE zenit)
add_test(NAME benchmark_string COMMAND benchmark_string)
set_tests_properties(benchmark_string PROPERTIES LABELS "benchmark")

# Linked list throughput
add_executable(benchmark_list benchmark_list.c)
target_link_libraries(benchmark_list PRIVATE zenit)
add_test(NAME benchmark_list COMMAND benchmark_list)
set_tests_properties(benchmark_list PROPERTIES LABELS "benchmark")

# Heap throughput
add_executable(benchmark_heap benchmark_heap.c)
target_link_libraries(benchmark_heap PRIVATE zenit)
add_test(NAME benchmark_heap COMMAND benchmark_heap)
set_tests_properties(benchmark_heap PROPERTIES LABELS "benchmark")

# Deque throughput
add_executable(benchmark_deque benchmark_deque.c)
target_link_libraries(benchmark_deque PRIVATE zenit)
add_test(NAME benchmark_deque COMMAND benchmark_deque)
set_tests_properties(benchmark_deque PROPERTIES LABELS "benchmark")

# Bitset throughput
add_executable(benchmark_bitset benchmark_bitset.c)
target_link_libraries(benchmark_bitset PRIVATE zenit)
add_test(NAME benchmark_bitset COMMAND benchmark_bitset)
set_tests_properties(benchmark_bitset PROPERTIES LABELS "benchmark")

# JSON throughput
add_executable(benchmark_json benchmark_json.c)
target_link_libraries(benchmark_json PRIVATE zenit)
add_test(NAME benchmark_json COMMAND benchmark_json)
set_tests_properties(benchmark_json PROPERTIES LABELS "benchmark")

# Base64 throughput
add_executable(benchmark_base64 benchmark_base64.c)
target_link_libraries(benchmark_base64 PRIVATE zenit)
add_test(NAME benchmark_base64 COMMAND benchmark_base64)
set_tests_properties(benchmark_base64 PROPERTIES LABELS "benchmark")

# Hex throughput
add_executable(benchmark_hex benchmark_hex.c)
target_link_libraries(benchmark_hex PRIVATE zenit)
add_test(NAME benchmark_hex COMMAND benchmark_hex)
set_tests_properties(benchmark_hex PROPERTIES LABELS "benchmark")

# URI throughput
add_executable(benchmark_uri benchmark_uri.c)
target_link_libraries(benchmark_uri PRIVATE zenit)
add_test(NAME benchmark_uri COMMAND benchmark_uri)
set_tests_properties(benchmark_uri PROPERTIES LABELS "benchmark")

# String utilities throughput
add_executable(benchmark_str benchmark_str.c)
target_link_libraries(benchmark_str PRIVATE zenit)
add_test(NAME benchmark_str COMMAND benchmark_str)
set_tests_properties(benchmark_str PROPERTIES LABELS "benchmark")

# Stack throughput
add_executable(benchmark_stack benchmark_stack.c)
target_link_libraries(benchmark_stack PRIVATE zenit)
add_test(NAME benchmark_stack COMMAND benchmark_stack)
set_tests_properties(benchmark_stack PROPERTIES LABELS "benchmark")

# Queue throughput
add_executable(benchmark_queue benchmark_queue.c)
target_link_libraries(benchmark_queue PRIVATE zenit)
add_test(NAME benchmark_queue COMMAND benchmark_queue)
set_tests_properties(benchmark_queue PROPERTIES LABELS "benchmark")

# Timer throughput
add_executable(benchmark_timer benchmark_timer.c)
target_link_libraries(benchmark_timer PRIVATE zenit)
add_test(NAME benchmark_timer COMMAND benchmark_timer)
set_tests_properties(benchmark_timer PROPERTIES LABELS "benchmark")

# Object pool throughput
add_executable(benchmark_pool benchmark_pool.c)
target_link_libraries(benchmark_pool PRIVATE zenit)
add_test(NAME benchmark_pool COMMAND benchmark_pool)
set_tests_properties(benchmark_pool PROPERTIES LABELS "benchmark")

# File I/O throughput
add_executable(benchmark_io benchmark_io.c)
target_link_libraries(benchmark_io PRIVATE zenit)
add_test(NAME benchmark_io COMMAND benchmark_io)
set_tests_properties(benchmark_io PROPERTIES LABELS "benchmark")

# Sort throughput
add_executable(benchmark_sort benchmark_sort.c)
target_link_libraries(benchmark_sort PRIVATE zenit)
add_test(NAME benchmark_sort COMMAND benchmark_sort)
set_tests_properties(benchmark_sort PROPERTIES LABELS "benchmark")

# Allocator throughput
add_executable(benchmark_allocator benchmark_allocator.c)
target_link_libraries(benchmark_allocator PRIVATE zenit)
add_test(NAME benchmark_allocator COMMAND benchmark_allocator)
set_tests_properties(benchmark_allocator PROPERTIES LABELS "benchmark")

# Result (error string) throughput
add_executable(benchmark_result benchmark_result.c)
target_link_libraries(benchmark_result PRIVATE zenit)
add_test(NAME benchmark_result COMMAND benchmark_result)
set_tests_properties(benchmark_result PROPERTIES LABELS "benchmark")

if(LIBZENIT_BUILD_COVERAGE)
  foreach(tgt benchmark_version benchmark_state benchmark_arena benchmark_ring benchmark_vector benchmark_map benchmark_set benchmark_string benchmark_list benchmark_heap benchmark_deque benchmark_bitset benchmark_json benchmark_base64 benchmark_hex benchmark_uri benchmark_str benchmark_sort benchmark_stack benchmark_queue benchmark_timer benchmark_pool benchmark_io benchmark_allocator benchmark_result)
    target_compile_options(${tgt} PRIVATE --coverage -g -O0)
    target_link_options(${tgt} PRIVATE --coverage)
  endforeach()
endif()

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
  COMMAND "$<TARGET_FILE:benchmark_set>"
  COMMAND "$<TARGET_FILE:benchmark_string>"
  COMMAND "$<TARGET_FILE:benchmark_list>"
  COMMAND "$<TARGET_FILE:benchmark_heap>"
  COMMAND "$<TARGET_FILE:benchmark_deque>"
  COMMAND "$<TARGET_FILE:benchmark_bitset>"
  COMMAND "$<TARGET_FILE:benchmark_json>"
  COMMAND "$<TARGET_FILE:benchmark_base64>"
  COMMAND "$<TARGET_FILE:benchmark_hex>"
  COMMAND "$<TARGET_FILE:benchmark_uri>"
  COMMAND "$<TARGET_FILE:benchmark_str>"
  COMMAND "$<TARGET_FILE:benchmark_sort>"
  COMMAND "$<TARGET_FILE:benchmark_stack>"
  COMMAND "$<TARGET_FILE:benchmark_queue>"
  COMMAND "$<TARGET_FILE:benchmark_timer>"
  COMMAND "$<TARGET_FILE:benchmark_pool>"
  COMMAND "$<TARGET_FILE:benchmark_io>"
  COMMAND "$<TARGET_FILE:benchmark_allocator>"
  COMMAND "$<TARGET_FILE:benchmark_result>"
  COMMAND ${CMAKE_COMMAND} -E echo ""
  COMMAND ${CMAKE_COMMAND} -E echo "===== Done ====="
  USES_TERMINAL
)
