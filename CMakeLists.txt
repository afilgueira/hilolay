cmake_minimum_required(VERSION 3.8)

project(ults C ASM)

set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES ult_context_switch.S ults.c)

add_executable(ults ${SOURCE_FILES})
