#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

uniform float u_tileScale = 0.02;

out VS_OUT {
    vec3 worldPos;
    vec4 clipPos;
    vec2 waterUV;
} vs_out;

void main() 
{
    vec4 world = u_model * vec4(aPos, 1.0);
    vs_out.worldPos = world.xyz;

    vec4 clip = u_proj * u_view * world;
    vs_out.clipPos = clip;
    vs_out.waterUV = vs_out.worldPos.xz * u_tileScale;

    gl_Position = clip;
}
