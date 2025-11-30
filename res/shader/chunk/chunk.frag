#version 460 core

in vec2 UV;

uniform sampler2D u_atlas;

out vec4 FragColor;

void main()
{
    FragColor = texture(u_atlas, UV);
}
