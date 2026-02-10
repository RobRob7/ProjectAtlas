#version 460 core

layout (location = 0) in uint combined;

uniform vec3 u_chunkOrigin;
uniform mat4 u_view;
uniform mat4 u_proj;

// constants
const vec3[6] normalSample =
{
    { 1,  0,  0},
    {-1,  0,  0},
    { 0,  1,  0},
    { 0, -1,  0},
    { 0,  0,  1},
    { 0,  0, -1}
};

out VS_OUT {
    vec3 normalVS;
} vs_out;

void main()
{
    uint combinedCopy = combined;

    // skip tileX, tileY
    combinedCopy >>= 10;
    // derive normal index
    vs_out.normalVS = normalize(mat3(u_view) * normalSample[combinedCopy & 7]);
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
    gl_Position = u_proj * u_view * vec4(world, 1.0);
}


