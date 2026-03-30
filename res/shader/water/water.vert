#version 460 core

layout (location = 0) in vec3 aPos;

layout (std140, set = 0, binding = 0) uniform UBO
{
    // vert
    mat4 u_lightSpaceMatrix;
    mat4 u_model;
    mat4 u_view;
    mat4 u_proj;

    vec4 u_tileScale_pad;

    // frag
    float u_time;
    float u_distortStrength;
    float u_waveSpeed;
    int u_useShadowMap;

    float u_near;
    float u_far;
    vec2 u_screenSize;

    vec3 u_viewPos;
    int _pad0;

    vec3 u_lightDir;
    int _pad1;
    
    vec3 u_lightColor;
    float u_ambientStrength;
};

#ifdef VULKAN
layout(push_constant) uniform WaterPushConstants
{
    mat4 u_model;
} pc;
#endif

layout (location = 0) out VS_OUT {
    vec3 worldPos;
    vec4 clipPos;
    vec2 waterUV;
    vec4 FragPosLightSpace;
} vs_out;

void main() 
{
    #ifdef VULKAN
        vec4 world = pc.u_model * vec4(aPos, 1.0);
    #else
        vec4 world = u_model * vec4(aPos, 1.0);
    #endif

    vs_out.worldPos = world.xyz;

    vec4 clip = u_proj * u_view * world;
    vs_out.clipPos = clip;
    vs_out.waterUV = vs_out.worldPos.xz * u_tileScale_pad.x;

    // shadow - lsm
    vs_out.FragPosLightSpace = u_lightSpaceMatrix * vec4(vs_out.worldPos, 1.0);

    gl_Position = clip;
}
