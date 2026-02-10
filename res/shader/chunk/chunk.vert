#version 460 core

layout (location = 0) in uint combined;

uniform vec3 u_chunkOrigin;
uniform mat4 u_view;
uniform mat4 u_proj;
uniform vec4 u_clipPlane;

out vec2 UV;
out vec3 FragWorldPos;
out vec3 Normal;

// constants
const int ATLAS_COLS = 32;
const int ATLAS_ROWS = 32;

const vec2[4] uvSample = 
{ 
    {0, 0},
    {0, 1},
    {1, 1},
    {1, 0}
};

const vec3[6] normalSample =
{
    { 1,  0,  0},
    {-1,  0,  0},
    { 0,  1,  0},
    { 0, -1,  0},
    { 0,  0,  1},
    { 0,  0, -1}
};

// compute UV
vec2 atlasUV(uint tileX, uint tileY)
{
	float tileW = 1.0f / ATLAS_COLS;
	float tileH = 1.0f / ATLAS_ROWS;

    vec2 uv = {
        (tileX + uvSample[gl_VertexID % 4].x) * tileW,
		(tileY + uvSample[gl_VertexID % 4].y) * tileH
    };

    return uv;
} // end of atlasUV()

void main()
{
    uint combinedCopy = combined;
    // derive tileY
    uint tileY = combinedCopy & 31;
    combinedCopy >>= 5;
    // derive tileX
    uint tileX = combinedCopy & 31;
    combinedCopy >>= 5;

    // derive UV
    UV = atlasUV(tileX, tileY);

    // derive normal index
    Normal = normalSample[combinedCopy & 7];
    combinedCopy >>= 3;

    vec3 aPos;
    // derive aPos.x
    aPos.x = float(combinedCopy & 31);
    combinedCopy >>= 5;
    // derive aPos.y
    aPos.y = float(combinedCopy & 511);
    combinedCopy >>= 9;
    // derive aPos.z
    aPos.z = float(combinedCopy & 31);
    combinedCopy >>= 5;
    
    vec3 world = aPos + vec3(u_chunkOrigin);
    vec4 worldPos = vec4(world, 1.0);

    // clipping plane
    gl_ClipDistance[0] = dot(worldPos, u_clipPlane);
    FragWorldPos = world;
    
    gl_Position = u_proj * u_view * worldPos;
}
