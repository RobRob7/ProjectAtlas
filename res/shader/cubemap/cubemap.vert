#version 430 core

layout(location = 0)  in vec3 aPos;

uniform mat4 u_projection;
uniform mat4 u_view;

out vec3 TexCoords;

void main()
{
    TexCoords = aPos;
    gl_Position = u_projection * u_view * vec4(aPos, 1.0);
    // push to depth = 1.0 so it never occludes scene objects
    gl_Position = gl_Position.xyww;
}
