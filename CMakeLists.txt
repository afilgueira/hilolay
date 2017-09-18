cmake_minimum_required(VERSION 3.8)

project(ults C ASM)

set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES ult1000_th_context_switch.S ults.c threadminator/threadminator.c threadminator/threadminator.h)

add_executable(ults ${SOURCE_FILES})
