cmake_minimum_required(VERSION 3.3)
project(client)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES "Source Files/client.cpp" "Source Files/shared.h" "Source Files/shared.cpp" "Source Files/wrapper.cpp" "Source Files/wrapper.h")
add_executable(client ${SOURCE_FILES})