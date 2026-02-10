#version 460 core

layout (location = 0) in uint combined;

uniform mat4 u_model;
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
    mat4 modelView = u_view * u_model;

    uint combinedCopy = combined;

    // skip tileX, tileY
    combinedCopy >>= 10;
    // derive normal index
    vs_out.normalVS = normalize(mat3(modelView) * normalSample[combinedCopy & 7]);
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
    
    gl_Position = u_proj * modelView * vec4(aPos, 1.0);
}


