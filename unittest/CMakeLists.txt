link_libraries(kingpin_static)

add_executable(test_context test_context.cc)
add_executable(test_logger test_logger.cc)
add_executable(test_buffer test_buffer.cc)
add_executable(test_utils_listen test_utils_listen.cc)
add_executable(test_utils_connect test_utils_connect.cc)
add_executable(test_utils test_utils.cc)

enable_testing()
include(GoogleTest)
gtest_discover_tests(test_utils)
gtest_discover_tests(test_buffer)
