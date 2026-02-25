#version 460 core

layout (location = 0) in vec2 vUV;

layout (std140, set = 0, binding = 10) uniform UBO
{
    vec2 u_texelSize;
    vec2 _pad0;
};

layout (binding = 7) uniform sampler2D u_ssaoRaw;

layout (location = 0) out float FragAO;

void main()
{
    float result = 0.0;
    for (int x = -2; x <= 2; ++x)
    {
        for (int y = -2; y <= 2; ++y)
        {
            vec2 offset = vec2(x, y) * u_texelSize;
            result += texture(u_ssaoRaw, vUV + offset).r;
        } // end for
    } // end for

    FragAO = result / 25.0;
}
