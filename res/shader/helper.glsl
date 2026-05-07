#ifndef HELPER_GLSL
#define HELPER_GLSL

float LinearizeDepth(float z01, float nearPlane, float farPlane)
{
#ifdef VULKAN
    return (nearPlane * farPlane) /
           (farPlane - z01 * (farPlane - nearPlane));
#else
    float z = z01 * 2.0 - 1.0;

    return (2.0 * nearPlane * farPlane) /
           (farPlane + nearPlane - z * (farPlane - nearPlane));
#endif
} // end of LinearizeDepth()

vec3 ReconstructViewPos(vec2 uv, float z01, mat4 invProj)
{
    #ifdef VULKAN
        float z = z01;
    #else
        float z = z01 * 2.0 - 1.0;
    #endif

    vec4 clip = vec4(uv * 2.0 - 1.0, z, 1.0);

    vec4 view = invProj * clip;
    return view.xyz / view.w;
} // end of ReconstructViewPos()

vec3 ReconstructWorldPos(vec2 uv, float z01, mat4 invViewProj)
{
#ifdef VULKAN
    float z = z01;
#else
    float z = z01 * 2.0 - 1.0;
#endif

    vec4 clip = vec4(uv * 2.0 - 1.0, z, 1.0);

    vec4 worldPos = invViewProj * clip;
    return worldPos.xyz / worldPos.w;
} // end of reconstructWorldPos()

#endif