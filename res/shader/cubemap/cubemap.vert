#version 430 core
layout(location = 0)  in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    gl_Position = projection * view * vec4(aPos, 1.0);
    // push to depth = 1.0 so it never occludes scene objects
    gl_Position = gl_Position.xyww;
}
