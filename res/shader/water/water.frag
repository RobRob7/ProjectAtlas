#version 460 core

in VS_OUT {
    vec3 worldPos;
    vec3 normal;
    vec2 uv;
    vec4 clipPos;
} fs_in;

uniform sampler2D u_reflectionTex;
uniform sampler2D u_refractionTex;
uniform sampler2D u_refractionDepthTex;
uniform sampler2D u_dudvTex;

uniform float u_time;
uniform float u_distortStrength = 0.02;
uniform float u_waveSpeed = 0.03;

uniform float u_near;
uniform float u_far;

uniform sampler2D u_atlas;
uniform vec3 u_viewPos;
uniform vec3 u_lightPos;
uniform vec3 u_lightColor;
uniform float u_ambientStrength;

out vec4 FragColor;

float linearizeDepth(float z01)
{
    float z = z01 * 2.0 - 1.0;
    return (2.0 * u_near * u_far) / (u_far + u_near - z * (u_far - u_near));
}

void main()
{
    // NDC [-1, 1]
    vec2 ndc = fs_in.clipPos.xy / fs_in.clipPos.w;
    // uv [0, 1]
    vec2 uv = ndc * 0.5 + 0.5;
    uv = clamp(uv, 0.001, 0.999);

    vec3 N = normalize(fs_in.normal);
    vec3 V = normalize(u_viewPos - fs_in.worldPos);

    // --- DUDV --- //
    vec2 baseUV = fs_in.worldPos.xz * 0.02;
    float t = u_time;

    vec2 uv1 = fract(baseUV + vec2( t * u_waveSpeed,  t * u_waveSpeed * 0.5));
    vec2 uv2 = fract(baseUV * 1.73 + vec2(-t * u_waveSpeed * 0.6, t * u_waveSpeed * 0.9));

    vec2 d1 = texture(u_dudvTex, uv1).rg * 2.0 - 1.0;
    vec2 d2 = texture(u_dudvTex, uv2).rg * 2.0 - 1.0;

    // blend them (not 50/50 if you want variety)
    vec2 dudv = normalize(d1 + d2 * 0.7);
    float waveNormalStrength = 0.35;

    // dudv is in [-1,1], treat it as XZ slope
    vec3 waveN = normalize(vec3(-dudv.x * waveNormalStrength,
                                1.0,
                                -dudv.y * waveNormalStrength));

    // blend with mesh normal
    N = normalize(mix(N, waveN, 0.75));

    // scale distortion in screen space
    float ndv = clamp(dot(N, V), 0.0, 1.0);
    float waveBoost = mix(1.5, 0.6, ndv); // more at grazing angles
    vec2 distortion = dudv * (u_distortStrength * waveBoost);

    // distorted UVs for refraction/reflection
    float refrDist = 1.0;
    float reflDist = 0.35;
    vec2 refrUV = uv + distortion * refrDist;
    vec2 reflUV = uv + distortion * reflDist;

    // clamp after distortion
    refrUV = clamp(refrUV, 0.001, 0.999);
    reflUV = clamp(reflUV, 0.001, 0.999);

    // REFRACTION + REFLECTION
    vec3 refraction = texture(u_refractionTex, refrUV).rgb;
    vec3 reflection = texture(u_reflectionTex, reflUV).rgb;

    float fresnel = pow(1.0 - ndv, 5.0);
    fresnel = mix(0.02, 0.98, fresnel);

    // SHORELINE
    float sceneDepth01 = texture(u_refractionDepthTex, uv).r;
    float sceneDepth   = linearizeDepth(sceneDepth01);

    float waterDepth01  = gl_FragCoord.z;
    float waterDepth    = linearizeDepth(waterDepth01);

    float thickness = max(sceneDepth - waterDepth, 0.0);

    float edgeFade = clamp(thickness / 1.5, 0.0, 1.0);
    float alpha = mix(0.1, 0.85, edgeFade);

    // DEPTH ABSORPTION
    vec3 deepColor = vec3(0.0, 0.25, 0.35);
    float absorb = clamp(thickness / 8.0, 0.0, 1.0);

    // FOG
    float fogTune = 0.15;
    float fog = clamp(1.0 - exp(-thickness * fogTune), 0.0, 1.0);

    // tune refraction
    refraction = mix(refraction, deepColor, absorb);
    refraction = mix(refraction, deepColor, fog);

    vec3 base = mix(refraction, reflection, fresnel);

    // LIGHTING
    vec3 waterTint = vec3(0.0, 0.1, 0.3);

    // light cube
    vec3 L = normalize(u_lightPos - fs_in.worldPos);

    // add attenuation
    float distance = length(u_lightPos - fs_in.worldPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);

    // ambient + diffuse tint
    vec3 ambient = u_ambientStrength * waterTint;
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = waterTint * diff * 0.25;
    diffuse *= attenuation;

    // blinn-phong specular
    vec3 H = normalize(L + V);
    float shininess = 64.0;
    float specStrength = 0.25;
    float spec = pow(max(dot(N, H), 0.0), shininess);
    vec3 specular = u_lightColor * spec * specStrength;
    specular *= attenuation;
    specular *= fresnel;

    // world ambient strength tune
    float env = clamp(u_ambientStrength, 0.0, 1.0);
    base *= mix(0.05, 1.0, env);

    float specEnv = mix(0.2, 1.0, env);
    spec *= specEnv;

    // final color
    vec3 color = base + (ambient + diffuse + specular);
    FragColor = vec4(color, alpha);
}
