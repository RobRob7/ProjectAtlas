#version 460 core

uniform sampler2D u_atlas;
uniform vec3 u_viewPos;

out vec4 FragColor;

void main()
{
    FragColor = vec4(vec3(1.0), 1.0);
}
