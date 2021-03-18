cmake_minimum_required(VERSION 3.13)

include(GoogleTest)

add_executable(tests main_test.cpp)

target_include_directories(tests PUBLIC
    ..
    ext/googletest/include    
)

target_link_libraries(tests PUBLIC gtest gtest_main ${PROJECT_NAME})

gtest_discover_tests(tests)
add_test(NAME tests COMMAND tests)