#version 460 core

in VS_OUT {
    vec3 worldPos;
    vec4 clipPos;
    vec2 waterUV;
} fs_in;

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

layout (binding = 8) uniform sampler2D u_waterReflColorTex;
layout (binding = 9) uniform sampler2D u_waterRefrColorTex;
layout (binding = 10) uniform sampler2D u_waterRefrDepthTex;

layout (binding = 11) uniform sampler2D u_waterDUDVTex;
layout (binding = 12) uniform sampler2D u_waterNormalTex;

out vec4 FragColor;

float linearizeDepth(float z01)
{
    // z01 is [0,1] -> [-1,1]
    float z = z01 * 2.0 - 1.0;
    return (2.0 * u_near * u_far) / (u_far + u_near - z * (u_far - u_near));
} // end of linearizeDepth

void main()
{
    // [0, 1]
    vec2 uv = (fs_in.clipPos.xy / fs_in.clipPos.w) * 0.5 + 0.5;
    // vec2 uv = gl_FragCoord.xy / u_screenSize;
    uv = clamp(uv, 0.001, 0.999);

    // view dir
    vec3 V = normalize(u_viewPos - fs_in.worldPos);

    // ------- DISTORTION ------- //
    // [0, 1]
    vec2 baseUV = fs_in.waterUV;
    vec2 dudvUV1 = fract(baseUV + vec2( u_time * u_waveSpeed,  u_time * u_waveSpeed * 0.5));
    vec2 dudvUV2 = fract(baseUV * 1.73 + vec2(-u_time * u_waveSpeed * 0.6, u_time * u_waveSpeed * 0.9));

    // [-1, 1]
    vec2 distortion1 = texture(u_waterDUDVTex, dudvUV1).rg * 2.0 - 1.0;
    vec2 distortion2 = texture(u_waterDUDVTex, dudvUV2).rg * 2.0 - 1.0;

    // mix d1, d2
    float mixFactor = 0.7;
    vec2 dudv = mix(distortion1, distortion2, mixFactor);

    // water normal using dudv
    vec2 nUV1 = dudvUV1;
    vec2 nUV2 = dudvUV2; 
    nUV1 = fract(nUV1 + dudv * 0.05);
    nUV2 = fract(nUV2 + dudv * 0.05);

    vec3 n1 = texture(u_waterNormalTex, nUV1).rgb;
    vec3 n2 = texture(u_waterNormalTex, nUV2).rgb;
    vec3 nTex = mix(n1, n2, mixFactor);

	vec3 N = vec3(nTex.r * 2.0 - 1.0, nTex.b, nTex.g * 2.0 - 1.0);
	N = normalize(N);

    // scale distortion in SS
    float ndv = clamp(dot(N, V), 0.0, 1.0);
    float waveBoost = mix(1.5, 0.6, ndv);
    vec2 distortion = dudv * ((u_distortStrength / u_screenSize) * waveBoost);

    // fade distortion near screen edges to avoid clamp seams
    vec2 border = min(uv, 1.0 - uv);
    float edge = clamp(min(border.x, border.y) / 0.03, 0.0, 1.0);
    // distortion *= edge;

    float refrDist = 1.0;
    vec2 refrTexCoords = uv + distortion * refrDist;
    refrTexCoords = clamp(refrTexCoords, 0.0, 1.0);

    float reflDist = 0.35;
    vec2 reflDistortion = vec2(distortion.x, -distortion.y);
    vec2 reflTexCoords = vec2(uv.x, 1.0 - uv.y) + reflDistortion * reflDist;
    reflTexCoords = clamp(reflTexCoords, 0.0, 1.0);

    // refr, refl
    vec3 refraction = texture(u_waterRefrColorTex, refrTexCoords).rgb;
    vec3 reflection = texture(u_waterReflColorTex, reflTexCoords).rgb;

    // ------- DEPTH ------- //
    float sceneDepth01 = texture(u_waterRefrDepthTex, refrTexCoords).r;
    float sceneDepth   = linearizeDepth(sceneDepth01);

    float waterDepth01  = gl_FragCoord.z;
    float waterDepth    = linearizeDepth(waterDepth01);

    float thickness = max(sceneDepth - waterDepth, 0.0);
    // kill distortion along shoreline
    float shoreWidth = 20.0;
    float shore = smoothstep(0.0, shoreWidth, thickness);

    // apply shore
    distortion *= shore;

    // ------- FRESNEL ------- //
    // allow some reflections even when looking straight down at water
    float fresnel = pow(1.0 - ndv, 5.0);
    fresnel = mix(0.02, 0.98, fresnel);

    // ------- SHORELINE ------- //
    float edgeFade = clamp(thickness / 1.5, 0.0, 1.0);
    float clarity = mix(0.85, 0.15, edgeFade);

    // DEPTH ABSORPTION
    vec3 deepColor = vec3(0.0, 0.25, 0.35) * u_ambientStrength;
    float k = 0.2;
    float absorb = 1.0 - exp(-thickness * k);
    refraction = mix(refraction, deepColor, absorb);

    // ------- PRE-LIGHTING ------- //
    vec3 surface = mix(refraction, reflection, fresnel);
    vec3 base = mix(surface, refraction, clarity);

    // ------- LIGHTING ------- //
    vec3 waterTint = vec3(0.0, 0.1, 0.3);
    vec3 L = normalize(u_lightPos - fs_in.worldPos);

    float distance = length(u_lightPos - fs_in.worldPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);

    // AMBIENT
    vec3 ambient = waterTint * u_ambientStrength;

    // DIFFUSE
    vec3 diffuse = waterTint * u_lightColor * max(dot(N, L), 0.0) * attenuation;

    // SPECULAR
    vec3 H = normalize(L + V);
    float shininess = 16.0;
    float spec = pow(max(dot(N, H), 0.0), shininess);
    float specStrength = 0.50;
    vec3 specular = u_lightColor * spec * specStrength;

    // output color
    vec3 finalColor = base + (ambient + diffuse + specular);
    FragColor = vec4(finalColor, 1.0);
}
