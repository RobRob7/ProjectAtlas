#version 460 core

in vec2 UV;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D u_atlas;
uniform vec3 u_viewPos;
uniform vec3 u_lightPos;
uniform vec3 u_lightColor;
uniform float u_ambientStrength;

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

    float ao = 1.0;
    if (u_useSSAO)
    {
        vec2 ssUV = gl_FragCoord.xy / u_screenSize;
        ao = texture(u_ssao, ssUV).r;
    }

    // add attenuation
    float distance = length(u_lightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);

    // ambient
    vec3 ambient = u_ambientStrength * texColor.rgb * ao;

    // diffuse
    vec3 lightDir = normalize(u_lightPos - FragPos);
    vec3 normal = normalize(Normal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = u_lightColor * diff * texColor.rgb;

    // specular
    vec3 viewDir = normalize(u_viewPos-FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
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

    // ambient receives no attenuation
    FragColor = vec4(ambient + (direct) * attenuation, 1.0);
}
