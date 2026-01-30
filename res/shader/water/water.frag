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
    // REFRACTION + REFLECTION
    // NDC [-1, 1]
    vec2 ndc = fs_in.clipPos.xy / fs_in.clipPos.w;
    // uv [0, 1]
    vec2 uv = ndc * 0.5 + 0.5;
    uv = clamp(uv, 0.001, 0.999);

    vec3 refraction = texture(u_refractionTex, uv).rgb;
    vec3 reflection = texture(u_reflectionTex, uv).rgb;

    vec3 N = normalize(fs_in.normal);
    vec3 V = normalize(u_viewPos - fs_in.worldPos);

    float ndv = clamp(dot(N, V), 0.0, 1.0);
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

    // final color
    vec3 color = base + (ambient + diffuse + specular);
    // world ambient strength tune
    float env = clamp(u_ambientStrength, 0.15, 1.0);
    color *= env;
    FragColor = vec4(color, alpha);
}
