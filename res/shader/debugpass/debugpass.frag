#version 460 core

layout (location = 0) in vec2 vUV;

layout (std140, set = 0, binding = 3) uniform UBO
{
    int u_mode;
    float u_near;
    float u_far;
    float _pad0;
};

layout (binding = 0) uniform sampler2D u_gNormal;
layout (binding = 1) uniform sampler2D u_gDepth;

layout (location = 0) out vec4 FragColor;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * u_near * u_far) / (u_far + u_near - z * (u_far - u_near));
}

void main()
{
    // normal
    if (u_mode == 1)
    {
        vec3 n = texture(u_gNormal, vUV).xyz;
        FragColor = vec4(n * 0.5 + 0.5, 1.0);
        return;
    }

    // depth
    if (u_mode == 2)
    {
        float d = texture(u_gDepth, vUV).r;
        float lin = LinearizeDepth(d);

        // visualize depth
        float vis = clamp(lin / u_far, 0.0, 1.0);
        FragColor = vec4(vec3(vis), 1.0);
        return;
    }
    FragColor = vec4(1.0, 0.0, 1.0, 1.0);
}
