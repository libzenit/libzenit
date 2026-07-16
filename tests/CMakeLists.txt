add_executable(test_libzenit
  test_version.c
)

target_link_libraries(test_libzenit PRIVATE zenit)

if(LIBZENIT_BUILD_COVERAGE)
  target_compile_options(test_libzenit PRIVATE --coverage -g -O0)
  target_link_options(test_libzenit PRIVATE --coverage)
endif()

add_test(NAME test_libzenit COMMAND test_libzenit)
