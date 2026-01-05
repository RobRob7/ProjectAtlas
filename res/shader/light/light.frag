#version 460 core

uniform sampler2D u_atlas;
uniform vec3 u_viewPos;
uniform vec3 u_color;

out vec4 FragColor;

void main()
{
    FragColor = vec4(u_color, 1.0);
}
