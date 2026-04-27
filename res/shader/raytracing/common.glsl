#ifndef COMMON_GLSL
#define COMMON_GLSL

#define SHADOW_RAY 1

struct RayPayload
{
    vec3 color;
    float depth;
    int shadowed;
    int rayType;
};

#endif