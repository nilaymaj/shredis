cmake_minimum_required(VERSION 3.25)
project(shredis)

set(CMAKE_CXX_STANDARD 17)

add_executable(shredis_server main.cpp Connection.cpp Connection.h Server.cpp Server.h Common.h Cursor.cpp Cursor.h DataStore.cpp DataStore.h AVLNode.cpp AVLNode.h)
add_executable(shredis_client client.cpp)
add_executable(shredis_test TestDataStore.cpp DataStore.h DataStore.cpp AVLNode.cpp AVLNode.h)
