cmake_minimum_required(VERSION 3.13)
project(cppmildew)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -g -O0")
enable_testing()
add_library(${PROJECT_NAME} 
    "mildew/cpp/object.cpp"
    "mildew/cpp/utf.cpp"
    "mildew/lexer.cpp"
    "mildew/nodes.cpp"
    "mildew/types/any.cpp"
    "mildew/types/object.cpp"
    "mildew/util/regex.cpp"
)
add_subdirectory(run)
add_subdirectory("ext/googletest")
add_subdirectory(tests)
