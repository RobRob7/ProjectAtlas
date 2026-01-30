#version 460 core

in vec2 UV;
in vec3 FragPos;
in vec3 Normal;
in float ClipDistance;

uniform sampler2D u_atlas;
uniform vec3 u_viewPos;
uniform vec3 u_lightPos;
uniform vec3 u_lightColor;
uniform float u_ambientStrength;
uniform bool u_useClipPlane;

uniform vec2 u_screenSize;
uniform bool u_useSSAO;
uniform sampler2D u_ssao;

out vec4 FragColor;

void main()
{
    vec4 texColor = texture(u_atlas, UV);
    if (texColor.a < 0.1)
    {
        discard;
    }

    if (u_useClipPlane && ClipDistance < 0.0)
    {
        discard;
    }

    float ao = 1.0;
    if (u_useSSAO)
    {
        vec2 ssUV = gl_FragCoord.xy / u_screenSize;
        ao = texture(u_ssao, ssUV).r;
    }

    // add attenuation
    float dist = length(u_lightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.09 * dist + 0.032 * dist * dist);

    // ambient
    vec3 ambient = u_ambientStrength * texColor.rgb * ao;

    // diffuse
    vec3 lightDir = normalize(u_lightPos - FragPos);
    vec3 normal = normalize(Normal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = u_lightColor * diff * texColor.rgb;

    // specular
    vec3 viewDir = normalize(u_viewPos - FragPos);
    float spec = 0.0;
    // blinn-phong
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float shininess = 32.0;
    float specStrength = 0.08;
    spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specular = u_lightColor * spec * specStrength * texColor.rgb;

    // allow AO to affect diffuse, specular lighting
    vec3 direct = (diffuse + specular);
    float aoStrength = pow(ao, 2.0);
    // max darkening ~60%
    direct *= mix(0.4, 1.0, aoStrength);

    // DISTANCE FOG
    // float d = distance(u_viewPos, FragPos);

    // vec3 fogColor = vec3(1.0, 1.0,1.0);
    // float fogStart = 175.0;
    // float fogEnd = 400.0;
    // float fogFactor = clamp((fogEnd - d) / (fogEnd - fogStart), 0.0, 1.0);

    // final color
    vec3 color = ambient + (direct) * attenuation;
    // color = mix(fogColor, color, fogFactor);
    FragColor = vec4(color, 1.0);
}
