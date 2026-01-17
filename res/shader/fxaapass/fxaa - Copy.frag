#version 460 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D u_sceneColorTex;
uniform vec2 u_inverseScreenSize;

// FXAA knobs
uniform float u_subPixelQuality;          // ~0.75
uniform float u_edgeThresholdMax;   // ~0.166
uniform float u_edgeThresholdMaxMin;// ~0.0833

// Luma helper (linear color assumed)
float luma(vec3 c) { return dot(c, vec3(0.299, 0.587, 0.114)); }

void main()
{
    vec3 rgbM  = texture(u_sceneColorTex, vUV).rgb;

    // vec2 offNW = vec2(-1.0, -1.0) * u_inverseScreenSize;
    // vec2 offNE = vec2( 1.0, -1.0) * u_inverseScreenSize;
    // vec2 offSW = vec2(-1.0,  1.0) * u_inverseScreenSize;
    // vec2 offSE = vec2( 1.0,  1.0) * u_inverseScreenSize;

    vec2 offNW = vec2(-1.0,  1.0) * u_inverseScreenSize;
    vec2 offNE = vec2( 1.0,  1.0) * u_inverseScreenSize;
    vec2 offSW = vec2(-1.0, -1.0) * u_inverseScreenSize;
    vec2 offSE = vec2( 1.0, -1.0) * u_inverseScreenSize;

    float lumaM  = luma(rgbM);
    float lumaNW = luma(texture(u_sceneColorTex, vUV + offNW).rgb);
    float lumaNE = luma(texture(u_sceneColorTex, vUV + offNE).rgb);
    float lumaSW = luma(texture(u_sceneColorTex, vUV + offSW).rgb);
    float lumaSE = luma(texture(u_sceneColorTex, vUV + offSE).rgb);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    float range   = lumaMax - lumaMin;

    // Early exit if not an edge
    float rangeThreshold = max(u_edgeThresholdMaxMin, lumaMax * u_edgeThresholdMax);
    if (range < rangeThreshold)
    {
        FragColor = vec4(rgbM, 1.0);
        return;
    }

    // Edge direction
    float dirX = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    float dirY =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    // Reduce directional sensitivity in low-contrast areas
    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * u_subPixelQuality), 1e-6);
    float rcpDirMin = 1.0 / (min(abs(dirX), abs(dirY)) + dirReduce);

    vec2 dir = clamp(vec2(dirX, dirY) * rcpDirMin, vec2(-8.0), vec2(8.0)) * u_inverseScreenSize;

    // Sample along edge
    vec3 rgbA = 0.5 * (
        texture(u_sceneColorTex, vUV + dir * (1.0/3.0 - 0.5)).rgb +
        texture(u_sceneColorTex, vUV + dir * (2.0/3.0 - 0.5)).rgb
    );

    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture(u_sceneColorTex, vUV + dir * (-0.5)).rgb +
        texture(u_sceneColorTex, vUV + dir * ( 0.5)).rgb
    );

    float lumaB = luma(rgbB);

    // Choose best blend
    if (lumaB < lumaMin || lumaB > lumaMax)
        FragColor = vec4(rgbA, 1.0);
    else
        FragColor = vec4(rgbB, 1.0);
}
