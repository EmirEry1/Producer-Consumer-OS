cmake_minimum_required(VERSION 3.24)
project(New_Files C)

set(CMAKE_C_STANDARD 23)

include_directories(.)

add_executable(New_Files
        kitchen.c
        meal.h)
