#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;
uniform vec4 u_clipPlane;

out vec2 UV;
out vec3 FragPos;
out vec3 Normal;
out float ClipDistance;

void main()
{
    UV = aUV;

    vec4 worldPos = u_model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;

    Normal = aNormal;
    
    ClipDistance = dot(worldPos, u_clipPlane);
    gl_Position = u_proj * u_view * worldPos;
}


