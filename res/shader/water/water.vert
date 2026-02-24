#version 460 core

layout (location = 0) in vec3 aPos;

layout (std140, binding = 11) uniform UBO
{
    // vert
    mat4 u_model;
    mat4 u_view;
    mat4 u_proj;

    vec4 u_tileScale_pad;

    // frag
    float u_time;
    float u_distortStrength;
    float u_waveSpeed;
    float _pad_waves;

    float u_near;
    float u_far;
    vec2 u_screenSize;

    vec3 u_viewPos;
    int _pad0;

    vec3 u_lightPos;
    int _pad1;
    
    vec3 u_lightColor;
    float u_ambientStrength;
};

out VS_OUT {
    vec3 worldPos;
    vec4 clipPos;
    vec2 waterUV;
} vs_out;

void main() 
{
    vec4 world = u_model * vec4(aPos, 1.0);
    vs_out.worldPos = world.xyz;

    vec4 clip = u_proj * u_view * world;
    vs_out.clipPos = clip;
    vs_out.waterUV = vs_out.worldPos.xz * u_tileScale_pad.x;

    gl_Position = clip;
}
