add_executable(run main.cpp)
target_include_directories(run PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(run PUBLIC ${PROJECT_NAME})
