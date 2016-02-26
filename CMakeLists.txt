cmake_minimum_required(VERSION 3.3)
project(testproject)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES "Source Files/client.cpp" "Source Files/server.cpp")
add_executable(testproject ${SOURCE_FILES})