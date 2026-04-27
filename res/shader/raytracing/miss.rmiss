#version 460 core
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "common.glsl"

layout(location = 0) rayPayloadInEXT RayPayload payload;

void main()
{
    // shadow miss - light is visible
    if (payload.rayType == 1)
    {
        payload.shadowed = 0;
        return;
    }
    
    // primary miss
    payload.color = vec3(0.45, 0.65, 1.0);
    payload.depth = 1e30;
}