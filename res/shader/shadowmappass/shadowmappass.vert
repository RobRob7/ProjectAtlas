#version 460 core

layout (location = 0) in uint combined;

layout (std140, set = 0, binding = 0) uniform UBO
{
	mat4 u_lightSpaceMatrix;

	vec3 u_chunkOrigin;
	float _pad0;
} ubo;

#ifdef VULKAN
layout (push_constant) uniform PushConstants
{
    vec4 u_chunkOrigin;
} pc;
#endif

void main()
{
    uint combinedCopy = combined;

    // skip uv index, tileX, tileY
    combinedCopy >>= 12;
    // derive normal index
    uint n = combinedCopy & 7u;
    n = min(n, 5u);
    combinedCopy >>= 3;

    vec3 aPos;
    // derive aPos.x
    aPos.x = float(combinedCopy & 15);
    combinedCopy >>= 4;
    // derive aPos.y
    aPos.y = float(combinedCopy & 511);
    combinedCopy >>= 9;
    // derive aPos.z
    aPos.z = float(combinedCopy & 15);
    combinedCopy >>= 4;

    #ifdef VULKAN
        vec3 world = aPos + vec3(pc.u_chunkOrigin);
    #else
        vec3 world = aPos + vec3(ubo.u_chunkOrigin);
    #endif

    gl_Position = ubo.u_lightSpaceMatrix * vec4(world, 1.0);
}


