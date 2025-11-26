cmake_minimum_required(VERSION 3.20)
project(stb)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE include "${CMAKE_CURRENT_SOURCE_DIR}/")