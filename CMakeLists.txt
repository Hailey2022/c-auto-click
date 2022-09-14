cmake_minimum_required(VERSION 3.23)
project(auto_click C)

set(CMAKE_C_STANDARD 99)

add_executable(auto_click WIN32 main.c)
add_link_options(auto_click PRIVATE -mwindows)
