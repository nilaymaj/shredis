cmake_minimum_required(VERSION 3.25)
project(shredis)

set(CMAKE_CXX_STANDARD 17)

add_executable(shredis main.cpp server.cpp client.cpp)
