cmake_minimum_required(VERSION 3.31)
project(queues)

set(CMAKE_CXX_STANDARD 17)

add_executable(queues src/main.cpp)
target_include_directories(queues PRIVATE include)