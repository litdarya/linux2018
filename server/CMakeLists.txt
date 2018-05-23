cmake_minimum_required(VERSION 3.9)
project(fs_ext2_like C)

set(CMAKE_C_STANDARD 99)

add_executable(fs_ext2_like main.c iNode.h file.h file.c iNode.c main.h Serializer.c cmake-build-debug/client.c)