#version 430 core

in vec3 TexCoords;

out vec4 FragColor;

layout (binding = 13) uniform samplerCube u_skyboxTex;

void main()
{
    FragColor = texture(u_skyboxTex, TexCoords);
}

