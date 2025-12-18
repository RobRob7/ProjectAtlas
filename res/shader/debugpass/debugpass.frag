#version 460 core

in vec2 vUV;

uniform sampler2D u_normal;
uniform sampler2D u_depth;

uniform int u_mode;
uniform float u_near;
uniform float u_far;

out vec4 FragColor;

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
        vec3 n = texture(u_normal, vUV).xyz;
        FragColor = vec4(n * 0.5 + 0.5, 1.0);
        return;
    }

    // depth
    if (u_mode == 2)
    {
        float d = texture(u_depth, vUV).r;
        float lin = LinearizeDepth(d);

        // visualize depth
        float vis = clamp(lin / u_far, 0.0, 1.0);
        FragColor = vec4(vec3(vis), 1.0);
        return;
    }
    FragColor = vec4(1.0, 0.0, 1.0, 1.0);
}
