#version 430 core

in vec3 TexCoords;

out vec4 FragColor;

uniform samplerCube u_skybox;

void main()
{
    FragColor = texture(u_skybox, TexCoords);
}

