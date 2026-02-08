#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 u_model;
uniform mat3 u_normalMatrix;
uniform mat4 u_view;
uniform mat4 u_proj;
uniform vec4 u_clipPlane;

out vec2 UV;
out vec3 FragWorldPos;
out vec3 Normal;

void main()
{
    UV = aUV;

    vec4 worldPos = u_model * vec4(aPos, 1.0);
    // clipping plane
    gl_ClipDistance[0] = dot(worldPos, u_clipPlane);
    FragWorldPos = worldPos.xyz;

    Normal = u_normalMatrix * aNormal;
    
    gl_Position = u_proj * u_view * worldPos;
}
