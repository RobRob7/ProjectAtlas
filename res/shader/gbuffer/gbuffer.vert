#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

out VS_OUT {
    vec3 normalVS;
} vs_out;

void main()
{
    mat4 modelView = u_view * u_model;

    vs_out.normalVS = normalize(mat3(modelView) * aNormal);
    
    gl_Position = u_proj * modelView * vec4(aPos, 1.0);
}


