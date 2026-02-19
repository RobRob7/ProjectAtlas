# cmake minimum version req
cmake_minimum_required(VERSION 3.12)

# using MSVC compiler
if(MSVC)
	# static runtime
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# project (using C++)
project(AtlasVk LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find Vulkan SDK
find_package(Vulkan REQUIRED)

# GLFW no build, install
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# pull dependecy cmake lists
add_subdirectory(deps/glfw-3.4)				# window opener
add_subdirectory(deps/glad)					# opengl loader
add_subdirectory(deps/glm-1.01)				# math
add_subdirectory(deps/freetype-2.10.0)		# freetype
add_subdirectory(deps/stb)					# stb image
add_subdirectory(deps/imgui-1.92.5)			# imgui
add_subdirectory(deps/libnoise)				# libnoise

# VS set startup project main
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${CMAKE_PROJECT_NAME})

# copy res folder to cmake binary directory
file(COPY ${CMAKE_SOURCE_DIR}/res DESTINATION ${CMAKE_BINARY_DIR})

# collect all *.cpp under src/ into MY_SOURCES
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")


# create exe target
add_executable("${CMAKE_PROJECT_NAME}")
target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES})

# include header directories
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

# define preprocessor macro RESOURCES_PATH
target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/res/")
# define preprocessor macro SAVE_PATH
target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC SAVE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/world/") 

# link libs
target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE Vulkan::Vulkan glm glfw glad freetype stb imgui libnoise)
