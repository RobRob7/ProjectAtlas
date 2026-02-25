#version 430 core

layout (location = 0) in vec3 TexCoords;

layout (binding = 13) uniform samplerCube u_skyboxTex;

layout (location = 0) out vec4 FragColor;

void main()
{
    FragColor = texture(u_skyboxTex, TexCoords);
}

