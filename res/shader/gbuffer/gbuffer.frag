#version 460 core

in VS_OUT {
    vec3 normalVS;
} fs_in;

layout (location = 0) out vec3 gNormal;

void main()
{
    gNormal = normalize(fs_in.normalVS);
}
