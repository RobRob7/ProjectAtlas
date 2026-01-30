#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

out VS_OUT {
    vec3 worldPos;
    vec3 normal;
    vec2 uv;
    vec4 clipPos;
} vs_out;

void main() 
{
    vec4 world = u_model * vec4(aPos, 1.0);
    vs_out.worldPos = world.xyz;
    vs_out.normal = aNormal;
    vs_out.uv = aUV;

    vec4 clip = u_proj * u_view * world;
    vs_out.clipPos = clip;

    gl_Position = clip;
}
