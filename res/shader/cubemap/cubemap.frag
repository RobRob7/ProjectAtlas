#version 430 core

layout (location = 0) in vec3 TexCoords;

layout (std140, set = 0, binding = 0) uniform UBO
{
    // vert
    mat4 u_view;
    mat4 u_proj;

    // frag
    vec3 _pad0;
    float u_dayNightMix;
};

layout (binding = 1) uniform samplerCube u_nightSkyboxTex;
layout (binding = 2) uniform samplerCube u_daySkyboxTex;

layout (location = 0) out vec4 FragColor;

void main()
{
    vec4 nightTex = texture(u_nightSkyboxTex, TexCoords);
    vec4 dayTex = texture(u_daySkyboxTex, TexCoords);
    
    FragColor = mix(dayTex, nightTex, u_dayNightMix);
}

