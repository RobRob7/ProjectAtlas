#version 460 core

layout (location = 0) in uint combined;

layout (std140, set = 0, binding = 0) uniform UBO
{
    // vert
    vec3 u_chunkOrigin;
    float _pad0;

    mat4 u_view;
    mat4 u_proj;

    vec4 u_clipPlane;

    // frag
    vec3 u_viewPos;
    float _pad1;

    vec3 u_lightPos;
    float _pad2;

    vec3 u_lightColor;
    float u_ambientStrength;

    vec2 u_screenSize;
    int u_useSSAO;
    int _pad3;
};

flat out uvec2 Tile;
out vec2 TileUV;
out vec3 FragWorldPos;
out vec3 Normal;

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
    vec3 world = aPos + vec3(u_chunkOrigin);
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

    // clipping plane
    gl_ClipDistance[0] = dot(worldPos, u_clipPlane);
    
    gl_Position = u_proj * u_view * worldPos;
}
