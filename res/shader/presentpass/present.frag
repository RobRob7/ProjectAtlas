#version 460 core

layout (location = 0) in vec2 vUV;

layout (binding = 14) uniform sampler2D u_forwardColorTex;

layout (location = 0) out vec4 FragColor;

void main()
{
    vec3 color = texture(u_forwardColorTex, vUV).rgb;
    FragColor = vec4(color, 1.0);
}
 