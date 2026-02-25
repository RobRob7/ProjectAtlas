#version 460 core

in vec2 vUV;

layout (std140, binding = 10) uniform UBO
{
    vec2 u_texelSize;
    vec2 _pad0;
};

uniform sampler2D u_ao;

out float FragAO;

void main()
{
    float result = 0.0;
    for (int x = -2; x <= 2; ++x)
    {
        for (int y = -2; y <= 2; ++y)
        {
            vec2 offset = vec2(x, y) * u_texelSize;
            result += texture(u_ao, vUV + offset).r;
        } // end for
    } // end for

    FragAO = result / 25.0;
}
