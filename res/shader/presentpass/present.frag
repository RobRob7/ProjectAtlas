#version 460 core

in vec2 vUV;

uniform sampler2D u_sceneColorTex;

out vec4 FragColor;

void main()
{
    vec3 color = texture(u_sceneColorTex, vUV).rgb;
    FragColor = vec4(color, 1.0);
}
 