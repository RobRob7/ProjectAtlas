#version 460 core

in vec2 UV;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D u_atlas;
uniform vec3 u_viewPos;
uniform vec3 u_lightPos;
uniform vec3 u_lightColor;

out vec4 FragColor;

void main()
{
    vec4 texColor = texture(u_atlas, UV);
    if (texColor.a < 0.1)
    {
        discard;
    }

    // add attenuation
    float distance = length(u_lightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);

    // ambient
    vec3 ambient = 0.1 * u_lightColor * texColor.rgb;

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
    spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specular = u_lightColor * spec;

    // ambient receives no attenuation
    FragColor = vec4(ambient + (diffuse + specular) * attenuation, 1.0);
}
