cmake_minimum_required(VERSION 3.31)
project(queues)

set(CMAKE_CXX_STANDARD 17)

add_executable(queues src/main.cpp)
target_include_directories(queues PRIVATE include)

if(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")
endif()