#version 460 core

#include "../helper.glsl"

layout (location = 0) in vec2 vUV;

#define MAX_KERNEL_SIZE 64

layout (std140, set = 0, binding = 0) uniform UBO
{
    mat4 u_proj;
    mat4 u_invProj;

    vec2 u_noiseScale;
    float u_radius;
    float u_bias;

    int u_kernelSize;
    float _pad0;
	vec2 _pad1;
} ubo;

layout (std140, set = 0, binding = 1) uniform UBO1
{
    vec4 u_samples[MAX_KERNEL_SIZE];
} uboSamples;

layout (binding = 2) uniform sampler2D u_gNormal;
layout (binding = 3) uniform sampler2D u_gDepth;
layout (binding = 4) uniform sampler2D u_ssaoNoiseTex;

layout (location = 0) out float FragAO;

void main()
{
    float depth01 = texture(u_gDepth, vUV).r;
    if (depth01 >= 1.0)
    {
        FragAO = 1.0;
        return;
    }

    vec3 N = normalize(texture(u_gNormal, vUV).xyz);
    vec3 P = ReconstructViewPos(vUV, depth01, ubo.u_invProj);

    //
    vec3 rand = normalize(texture(u_ssaoNoiseTex, vUV * ubo.u_noiseScale).xyz * 2.0 - 1.0);

    vec3 T = normalize(rand - N * dot(rand, N));
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);

    float occlusion = 0.0;

    for (int i = 0; i < ubo.u_kernelSize; ++i)
    {
        vec3 sampleVS = P + (TBN * uboSamples.u_samples[i].xyz) * ubo.u_radius;

        // get screen space (depth)
        vec4 offset = ubo.u_proj * vec4(sampleVS, 1.0);
        offset.xyz /= offset.w;
        vec2 uv = offset.xy * 0.5 + 0.5;

        // discard if outside screen
        if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
        {
            continue;
        }

        float sampleDepth01 = texture(u_gDepth, uv).r;
        vec3 sampleDepthVS = ReconstructViewPos(uv, sampleDepth01, ubo.u_invProj);

        float range = smoothstep(0.0, 1.0, ubo.u_radius / abs(P.z - sampleDepthVS.z));

        float isOccluded = (sampleDepthVS.z >= sampleVS.z + ubo.u_bias) ? 1.0 : 0.0;
        occlusion += isOccluded * range;
    } // end for

    float ao = 1.0 - (occlusion / float(ubo.u_kernelSize));
    FragAO = clamp(ao, 0.0, 1.0);
}
