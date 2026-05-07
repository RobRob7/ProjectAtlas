#version 460 core

#include "../helper.glsl"

layout(location = 0) in vec2 vUV;

layout(std140, set = 0, binding = 0) uniform UBO
{
    mat4 u_invViewProj;
    mat4 u_lightSpaceMatrix;

    vec3 u_cameraPos;
	float u_fogDensity;

    vec2 u_nearFar;
    vec2 u_fogStartEnd;

    vec3 u_fogColor;
    int u_useVolFog;

    vec3 u_lightDir;
    float u_maxDistance;

    float u_ambStr;
    float u_scatteringStrength;
    float u_shadowBias;
    int u_sampleCount;
} ubo;

layout(binding = 1) uniform sampler2D ForwardColorTex;
layout(binding = 2) uniform sampler2D ForwardDepthTex;
layout(binding = 3) uniform sampler2D ShadowMapTex;

layout(location = 0) out vec4 FragColor;

float sampleShadowMap(vec3 worldPos)
{
    vec4 lightClip = ubo.u_lightSpaceMatrix * vec4(worldPos, 1.0);
    vec3 proj = lightClip.xyz / lightClip.w;

#ifdef VULKAN
    proj.xy = proj.xy * 0.5 + 0.5;
#else
    proj = proj * 0.5 + 0.5;
#endif

    if (proj.x < 0.0 || proj.x > 1.0 ||
        proj.y < 0.0 || proj.y > 1.0 ||
        proj.z < 0.0 || proj.z > 1.0)
    {
        return 1.0;
    }

    float closestDepth = texture(ShadowMapTex, proj.xy).r;
    float currentDepth = proj.z;

    return currentDepth - ubo.u_shadowBias <= closestDepth ? 1.0 : 0.0;
} // end of sampleShadowMap()

void main()
{
    if (ubo.u_useVolFog == 0)
    {
        vec3 color = texture(ForwardColorTex, vUV).rgb;

        float z01 = texture(ForwardDepthTex, vUV).r;
        float d = LinearizeDepth(z01, ubo.u_nearFar.x, ubo.u_nearFar.y);

        float fogFactor = clamp((ubo.u_fogStartEnd.y - d) / (ubo.u_fogStartEnd.y - ubo.u_fogStartEnd.x), 0.0, 1.0);
        fogFactor = pow(fogFactor, 0.6);

        vec3 outColor = mix(ubo.u_fogColor * ubo.u_ambStr, color, fogFactor);
        FragColor = vec4(outColor, 1.0);
    }
    else
    {
        vec3 sceneColor = texture(ForwardColorTex, vUV).rgb;
        float z01 = texture(ForwardDepthTex, vUV).r;

        vec3 endPos = ReconstructWorldPos(vUV, z01, ubo.u_invViewProj);

        vec3 rayStart = ubo.u_cameraPos;
        vec3 rayVec = endPos - rayStart;

        float rayLength = length(rayVec);
        rayLength = min(rayLength, ubo.u_maxDistance);

        vec3 rayDir = normalize(rayVec);

        int samples = max(ubo.u_sampleCount, 1);
        float stepSize = rayLength / float(samples);

        vec3 fogAccum = vec3(0.0);

        for (int i = 0; i < samples; ++i)
        {
            float t = (float(i) + 0.5) / float(samples);
            vec3 samplePos = rayStart + rayDir * rayLength * t;

            float shadow = sampleShadowMap(samplePos);

            float phase = mix(0.4, 1.0, max(dot(rayDir, normalize(ubo.u_lightDir)), 0.0));

            vec3 scattered = 
                ubo.u_fogColor *
                ubo.u_fogDensity *
                shadow *
                phase *
                ubo.u_scatteringStrength;
            fogAccum += scattered * stepSize;
        } // end for

        float d = LinearizeDepth(z01, ubo.u_nearFar.x, ubo.u_nearFar.y);
        float fogFactor = clamp(
            (ubo.u_fogStartEnd.y - d) /
            (ubo.u_fogStartEnd.y - ubo.u_fogStartEnd.x),
            0.0,
            1.0
        );

        vec3 distanceFog = mix(ubo.u_fogColor * ubo.u_ambStr, sceneColor, fogFactor);

        vec3 finalColor = distanceFog + fogAccum;

        FragColor = vec4(finalColor, 1.0);
    }
}
 