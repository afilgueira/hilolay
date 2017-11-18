cmake_minimum_required(VERSION 3.8)

project(ults C)

set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES ults.c hilolay/hilolay.c hilolay/hilolay.h)

add_executable(ults ${SOURCE_FILES})
