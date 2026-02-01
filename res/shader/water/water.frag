#version 460 core

in VS_OUT {
    vec3 worldPos;
    // vec3 normal;
    // vec2 uv;
    vec4 clipPos;
} fs_in;

uniform sampler2D u_reflectionTex;
uniform sampler2D u_refractionTex;
uniform sampler2D u_refractionDepthTex;
uniform sampler2D u_dudvTex;
uniform sampler2D u_normalTex;
uniform sampler2D u_ssaoTex;

uniform float u_time;
uniform float u_distortStrength = 0.02;
uniform float u_waveSpeed = 0.03;
uniform float u_waveStrength = 0.03;

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
    // [0,1]
    vec2 ndc = (fs_in.clipPos.xy / fs_in.clipPos.w) * 0.5 + 0.5;
    ndc = clamp(ndc, 0.001, 0.999);

    // AO
    float ao = texture(u_ssaoTex,  ndc).r;

    vec2 reflectTextCoords = vec2(ndc.x, 1.0 - ndc.y);
	vec2 refractTexCoords  = ndc;

    float tilingScale = 0.1;
    vec2 waterUV = fs_in.worldPos.xz * tilingScale;
    waterUV += vec2(u_time * u_waveSpeed, u_time * u_waveSpeed * 0.5);

    vec2 dudv = texture(u_dudvTex, waterUV).rg * 2.0 - 1.0;
    ivec2 sz = textureSize(u_refractionTex, 0);
    vec2 texel = 1.0 / vec2(sz);

    float maxPixels = 8.0;
    vec2 totalDistortion = dudv * (u_waveStrength * maxPixels) * texel;

	reflectTextCoords += totalDistortion;
	reflectTextCoords = clamp(reflectTextCoords, 0.001,0.999);

	refractTexCoords += totalDistortion;
	refractTexCoords = clamp(refractTexCoords, 0.001, 0.999);

	vec4 reflectColor = texture(u_reflectionTex, reflectTextCoords);
	vec4 refractColor = texture(u_refractionTex, refractTexCoords);
    float depthFade = clamp(
        (fs_in.worldPos.y - 65.0 + 2.0) / 6.0,
        0.0, 1.0
    );

float aoStrength = mix(0.2, 0.75, depthFade);
    refractColor.rgb *= mix(1.0, ao, aoStrength);

	vec3 viewVector = normalize(u_viewPos - fs_in.worldPos);
	float refractiveFactor = dot(viewVector, vec3(0.0, 1.0, 0.0));
	refractiveFactor = pow(refractiveFactor, 2.0);

	vec4 normalMapColor = texture(u_normalTex, waterUV);
	vec3 normal = vec3(normalMapColor.r * 2.0 - 1.0, normalMapColor.b, normalMapColor.g * 2.0 - 1.0);
	normal = normalize(normal);

	vec3 reflectedLight = reflect(normalize(fs_in.worldPos - u_lightPos), normal);
	float specular = max(dot(reflectedLight, viewVector), 0.0);
	specular = pow(specular, 20.0);
	vec3 specularHighlights = u_lightColor * specular * 0.6;

	FragColor = mix(reflectColor, refractColor, refractiveFactor);
	FragColor = mix(FragColor, vec4(0.0,0.03,0.5,1.0), 0.1) + vec4(specularHighlights,0.0);
}
