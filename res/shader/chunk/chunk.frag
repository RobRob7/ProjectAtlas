#version 460 core

in vec2 UV;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D u_atlas;
uniform vec3 u_viewPos;

out vec4 FragColor;

vec3 lightPos = vec3(11.6f, 75.8f, 0.9f);
vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);

void main()
{
    vec4 texColor = texture(u_atlas, UV);
    if (texColor.a < 0.1)
    {
        discard;
    }

    // add attenuation
    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);

    // ambient
    vec3 ambient = 0.02 * lightColor * texColor.rgb;

    // diffuse
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 normal = normalize(Normal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = lightColor * diff * texColor.rgb;

    // specular
    vec3 viewDir = normalize(u_viewPos-FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    // blinn-phong
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float shininess = 32.0;
    spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specular = lightColor * spec;

    // ambient receives no attenuation
    FragColor = vec4(ambient + (diffuse + specular) * attenuation, 1.0);
}
