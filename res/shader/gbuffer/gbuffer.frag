#version 460 core

layout (location = 0) in VS_OUT {
    vec3 normalVS;
} fs_in;

layout (location = 0) out vec4 gNormal;

void main()
{
    gNormal = vec4(normalize(fs_in.normalVS), 1.0);
}
