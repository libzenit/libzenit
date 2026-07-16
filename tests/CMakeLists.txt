add_executable(test_libzenit
  test_version.c
)

target_link_libraries(test_libzenit PRIVATE zenit)

add_test(NAME test_libzenit COMMAND test_libzenit)
