# create the imgui library target
add_library(imgui
    imgui.cpp
    imgui_draw.cpp
    imgui_widgets.cpp
    imgui_tables.cpp
    imgui_demo.cpp
    backends/imgui_impl_glfw.cpp
    backends/imgui_impl_opengl3.cpp
)

# include directories for ImGui (core + backends)
target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/backends
)

# tell ImGui that we’re using GLAD as the OpenGL loader
target_compile_definitions(imgui PUBLIC
    IMGUI_IMPL_OPENGL_LOADER_GLAD
)

# link ImGui against GLFW and GLAD targets that root CMakeList already builds
target_link_libraries(imgui PUBLIC
    glfw
    glad
)
