#version 460 core

in vec2 vUV;

layout (std140, binding = 5) uniform UBO
{
    vec2 u_inverseScreenSize;
    float u_edgeSharpnessQuality;
    float u_edgeThresholdMax;

    float u_edgeThresholdMin;
    float _pad0;
    vec2 _pad1;
};

uniform sampler2D u_sceneColorTex;

out vec4 FragColor;

// rgb to luma helper
float luma(vec3 rgb)
{
    return dot(rgb, vec3(0.299, 0.587, 0.114));
} // end of rgb2luma()

void main()
{
    vec2 offNW = vec2(-1.0,  1.0) * u_inverseScreenSize;
    vec2 offNE = vec2( 1.0,  1.0) * u_inverseScreenSize;
    vec2 offSW = vec2(-1.0, -1.0) * u_inverseScreenSize;
    vec2 offSE = vec2( 1.0, -1.0) * u_inverseScreenSize;

    float lumaNW = luma(texture(u_sceneColorTex, vUV + offNW).rgb);
    float lumaNE = luma(texture(u_sceneColorTex, vUV + offNE).rgb);
    float lumaSW = luma(texture(u_sceneColorTex, vUV + offSW).rgb);
    float lumaSE = luma(texture(u_sceneColorTex, vUV + offSE).rgb);

    vec3 rgbM  = texture(u_sceneColorTex, vUV).rgb;

    float lumaM = luma(rgbM);

    float lumaMaxNWSW = max(lumaNW, lumaSW);
    lumaNE += 1.0/384.0;
    float lumaMinNWSW = min(lumaNW, lumaSW);

    float lumaMaxNESE = max(lumaNE, lumaSE);
    float lumaMinNESE = min(lumaNE, lumaSE);

    float lumaMax = max(lumaMaxNESE, lumaMaxNWSW);
    float lumaMin = min(lumaMinNESE, lumaMinNWSW);

    float lumaMaxScaled = lumaMax * u_edgeThresholdMax;

    float lumaMinM = min(lumaMin, lumaM);
    float lumaMaxScaledClampled = max(u_edgeThresholdMin, lumaMaxScaled);
    float lumaMaxM = max(lumaMax, lumaM);
    float dirSWMinusNE = lumaSW - lumaNE;
    float lumaMaxSubMinM = lumaMaxM - lumaMinM;
    float dirSEMinusNW = lumaSE - lumaNW;

    if (lumaMaxSubMinM < lumaMaxScaledClampled)
    {
        FragColor = vec4(rgbM, 1.0);
        return;
    }

    vec2 dir;
    dir.x = dirSWMinusNE + dirSEMinusNW;
    dir.y = dirSWMinusNE - dirSEMinusNW;

    vec2 dir1 = normalize(dir);
    vec4 rgbN1 = texture(u_sceneColorTex, vUV - dir1 * u_inverseScreenSize);
    vec4 rgbP1 = texture(u_sceneColorTex, vUV + dir1 * u_inverseScreenSize);

    float dirAbsMinTimesC = min(abs(dir1.x), abs(dir1.y)) * u_edgeSharpnessQuality;
    vec2 dir2 = clamp(dir1.xy / dirAbsMinTimesC, -2.0, 2.0);

    vec4 rgbN2 = texture(u_sceneColorTex, vUV - dir2 * u_inverseScreenSize);
    vec4 rgbP2 = texture(u_sceneColorTex, vUV + dir2 * u_inverseScreenSize);

    vec4 rgbA = rgbN1 + rgbP1;
    vec4 rgbB = ((rgbN2 + rgbP2) * 0.25) + (rgbA * 0.25);

    float lumaB = luma(rgbB.rgb);
    if (lumaB < lumaMin || lumaB > lumaMax)
        FragColor = vec4(rgbA.rgb * 0.5, 1.0);
    else
        FragColor = vec4(rgbB.rgb, 1.0);
} // end of main
 