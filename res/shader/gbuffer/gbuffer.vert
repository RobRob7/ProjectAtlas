#version 460 core

layout (location = 0) in uint combined;

layout (std140, binding = 6) uniform UBO
{
	mat4 u_view;
	mat4 u_proj;

	vec3 u_chunkOrigin;
	float _pad0;
};

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

    // skip uv index, tileX, tileY
    combinedCopy >>= 12;
    // derive normal index
    uint n = combinedCopy & 7u;
    n = min(n, 5u);
    vs_out.normalVS = normalize(mat3(u_view) * normalSample[n]);
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

    vec3 world = aPos + vec3(u_chunkOrigin);
    gl_Position = u_proj * u_view * vec4(world, 1.0);
}


