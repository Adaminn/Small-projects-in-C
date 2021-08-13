cmake_minimum_required(VERSION 3.16)
project(hw06 C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(hw06
        cracker.c
        md5.c
        md5.h)
