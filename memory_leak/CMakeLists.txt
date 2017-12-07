cmake_minimum_required(VERSION 3.8)

project(main C)
include_directories(${SOURCE_FILES} ../hilolay)

set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES main.c ${SOURCE_FILES} ../hilolay/hilolay.c ${SOURCE_FILES} ../hilolay/hilolay.h)

add_executable(main ${SOURCE_FILES})
