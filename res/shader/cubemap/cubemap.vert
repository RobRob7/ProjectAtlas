#version 430 core

layout (location = 0) in vec3 aPos;

layout (std140, set = 0, binding = 0) uniform UBO
{
    // vert
    mat4 u_view;
    mat4 u_proj;

    // frag
    vec3 _pad0;
    float u_dayNightMix;
};

layout (location = 0) out vec3 TexCoords;

void main()
{
    TexCoords = aPos;
    gl_Position = u_proj * u_view * vec4(aPos, 1.0);
    // push to depth = 1.0 so it never occludes scene objects
    gl_Position = gl_Position.xyww;
}
