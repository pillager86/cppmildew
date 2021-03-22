cmake_minimum_required(VERSION 3.13)
project(cppmildew)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
enable_testing()
# find_package(Boost REQUIRED COMPONENTS fiber)
add_library(${PROJECT_NAME} 
    "cppd/object.cpp"
    "cppd/utf.cpp"
    "mildew/environment.cpp"
    "mildew/interpreter.cpp"
    "mildew/lexer.cpp"
    "mildew/nodes.cpp"
    "mildew/parser.cpp"
    "mildew/types/any.cpp"
    "mildew/types/array.cpp"
    "mildew/types/function.cpp"
    "mildew/types/object.cpp"
    "mildew/types/string.cpp"
    "mildew/util/regex.cpp"
)
# target_link_libraries(${PROJECT_NAME} PUBLIC Boost::context Boost::fiber)
add_subdirectory(run)
add_subdirectory("ext/googletest")
add_subdirectory(tests)
