#version 460 core

layout (location = 0) in vec3 aPos;

layout (std140, binding = 7) uniform UBO
{
    mat4 u_model;
    mat4 u_view;
    mat4 u_proj;
    vec4 u_color;
};

void main()
{
    gl_Position = u_proj * u_view * u_model * vec4(aPos, 1.0);
}
