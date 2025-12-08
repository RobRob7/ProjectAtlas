cmake_minimum_required(VERSION 3.12)
project(libnoise)

# Collect all .cpp files in src and its subdirectories
file(GLOB_RECURSE LIBNOISE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(libnoise STATIC ${LIBNOISE_SOURCES})


target_include_directories(libnoise PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
