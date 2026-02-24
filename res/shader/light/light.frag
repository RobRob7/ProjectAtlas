#version 460 core

layout (std140, binding = 7) uniform UBO
{
    mat4 u_model;
    mat4 u_view;
    mat4 u_proj;
    vec4 u_color;
};

out vec4 FragColor;

void main()
{
    FragColor = vec4(u_color.rgb, 1.0);
}
