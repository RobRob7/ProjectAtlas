#version 460 core

layout (location = 0) in uint combined;

layout (std140, set = 0, binding = 0) uniform UBO
{
    // vert
    mat4 u_lightSpaceMatrix;

    vec3 u_chunkOrigin;
    float _pad0;

    mat4 u_view;
    mat4 u_proj;

    vec4 u_clipPlane;

    // frag
    vec3 u_viewPos;
    float _pad1;

    vec3 u_lightDir;
    float _pad2;

    vec3 u_lightColor;
    float u_ambientStrength;

    vec2 u_screenSize;
    int u_useSSAO;
    int _pad3;
};

#ifdef VULKAN
layout(push_constant) uniform PushConstants
{
    vec4 u_chunkOrigin;
} pc;
#endif

layout (location = 0) flat out uvec2 Tile;
layout (location = 1) out vec2 TileUV;
layout (location = 2) out vec3 FragWorldPos;
layout (location = 3) out vec3 Normal;
layout (location = 4) out vec4 FragPosLightSpace;

const vec3[6] normalSample =
{
    { 1,  0,  0},
    {-1,  0,  0},
    { 0,  1,  0},
    { 0, -1,  0},
    { 0,  0,  1},
    { 0,  0, -1}
};

void main()
{
    uint combinedCopy = combined;
    // derive uv corner index
    uint uvIndex = combinedCopy & 3;
    combinedCopy >>= 2;

    // derive tileY
    uint tileY = combinedCopy & 31;
    combinedCopy >>= 5;
    // derive tileX
    uint tileX = combinedCopy & 31;
    combinedCopy >>= 5;

    // derive normal index
    uint normalIndex = combinedCopy & 7;
    Normal = normalSample[normalIndex];
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
    
    Tile = uvec2(tileX, tileY);

    #ifdef VULKAN
    vec3 world = aPos + vec3(pc.u_chunkOrigin);
    #else
    vec3 world = aPos + vec3(u_chunkOrigin);
    #endif
    
    vec4 worldPos = vec4(world, 1.0);
    FragWorldPos = world;

    // X faces use (z,y)
    if (normalIndex == 0u || normalIndex == 1u)
    {      
        TileUV = world.zy; // +/-X
    }
    // Y faces use (x,z)
    else if (normalIndex == 2u || normalIndex == 3u) 
    {
        TileUV = world.xz; // +/-Y
    }
    // Z faces use (x,y)
    else
    {
        TileUV = world.xy; // +/-Z
    }

    // shadow - lsm
    FragPosLightSpace = u_lightSpaceMatrix * vec4(FragWorldPos, 1.0);

    // clipping plane
    gl_ClipDistance[0] = dot(worldPos, u_clipPlane);
    
    gl_Position = u_proj * u_view * worldPos;
}
