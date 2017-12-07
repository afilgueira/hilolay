cmake_minimum_required(VERSION 3.8)

project(example C)
include_directories(${SOURCE_FILES} ../hilolay)

set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES example.c ${SOURCE_FILES} ../hilolay/hilolay.c ${SOURCE_FILES} ../hilolay/hilolay.h)

add_executable(example ${SOURCE_FILES})
