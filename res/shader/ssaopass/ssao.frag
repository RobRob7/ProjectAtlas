#version 460 core

in vec2 vUV;

#define MAX_KERNEL_SIZE 64

uniform sampler2D u_gNormal;
uniform sampler2D u_gDepth;
uniform sampler2D u_noise;

uniform mat4 u_proj;
uniform mat4 u_invProj;
uniform vec2 u_noiseScale;

uniform float u_radius;
uniform float u_bias;
uniform int u_kernelSize;

uniform vec3 u_samples[MAX_KERNEL_SIZE];

out float FragAO;

vec3 ReconstructViewPos(vec2 uv, float depth01)
{
    // depth01 in [0,1] -> [-1,1]
    float z = depth01 * 2.0 - 1.0;
    vec4 clip = vec4(uv * 2.0 - 1.0, z, 1.0);

    vec4 view = u_invProj * clip;
    return view.xyz / view.w;
}

void main()
{
    float depth01 = texture(u_gDepth, vUV).r;
    if (depth01 >= 1.0)
    {
        FragAO = 1.0;
        return;
    }

    vec3 N = normalize(texture(u_gNormal, vUV).xyz);
    vec3 P = ReconstructViewPos(vUV, depth01);

    //
    vec3 rand = normalize(texture(u_noise, vUV * u_noiseScale).xyz * 2.0 - 1.0);

    vec3 T = normalize(rand - N * dot(rand, N));
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);

    float occlusion = 0.0;

    for (int i = 0; i < u_kernelSize; ++i)
    {
        vec3 sampleVS = P + (TBN * u_samples[i]) * u_radius;

        // get screen space (depth)
        vec4 offset = u_proj * vec4(sampleVS, 1.0);
        offset.xyz /= offset.w;
        vec2 uv = offset.xy * 0.5 + 0.5;

        // discard if outside screen
        if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
        {
            continue;
        }

        float sampleDepth01 = texture(u_gDepth, uv).r;
        vec3 sampleDepthVS = ReconstructViewPos(uv, sampleDepth01);

        float range = smoothstep(0.0, 1.0, u_radius / abs(P.z - sampleDepthVS.z));

        float isOccluded = (sampleDepthVS.z >= sampleVS.z + u_bias) ? 1.0 : 0.0;
        occlusion += isOccluded * range;
    } // end for

    float ao = 1.0 - (occlusion / float(u_kernelSize));
    FragAO = clamp(ao, 0.0, 1.0);
}
