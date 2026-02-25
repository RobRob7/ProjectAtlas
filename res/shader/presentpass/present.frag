#version 460 core

in vec2 vUV;

layout (binding = 14) uniform sampler2D u_forwardColorTex;

out vec4 FragColor;

void main()
{
    vec3 color = texture(u_forwardColorTex, vUV).rgb;
    FragColor = vec4(color, 1.0);
}
 