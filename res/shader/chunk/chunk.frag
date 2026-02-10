#version 460 core

flat in uvec2 Tile;
in vec2 TileUV;
in vec3 FragWorldPos;
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

const float ATLAS_COLS = 32.0;
const float ATLAS_ROWS = 32.0;

vec2 atlasUV(uvec2 tile, vec2 local01)
{
    vec2 tileW = vec2(1.0 / ATLAS_COLS, 1.0 / ATLAS_ROWS);
    return (vec2(tile) + local01) * tileW;
}

void main()
{
    // get correct UV (due to greedy meshing)
    vec2 local = fract(TileUV);
    vec2 uv = atlasUV(Tile, local);

    vec4 texColor = texture(u_atlas, uv);

    // allow textures to be see-through
    // (like tree canopy texture)
    if (texColor.a < 0.1)
    {
        discard;
    }

    // SSAO
    float ao = 1.0;
    if (u_useSSAO)
    {
        vec2 ssUV = gl_FragCoord.xy / u_screenSize;
        ao = texture(u_ssao, ssUV).r;
    }

    // add attenuation
    float dist = length(u_lightPos - FragWorldPos);
    float attenuation = 1.0 / (1.0 + 0.09 * dist + 0.032 * dist * dist);

    // ambient
    vec3 ambient = u_ambientStrength * texColor.rgb * ao;

    // diffuse
    vec3 lightDir = normalize(u_lightPos - FragWorldPos);
    vec3 normal = normalize(Normal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = u_lightColor * diff * texColor.rgb;

    // specular
    vec3 viewDir = normalize(u_viewPos - FragWorldPos);
    float spec = 0.0;
    // blinn-phong
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float shininess = 32.0;
    float specStrength = 0.08;
    spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specular = u_lightColor * spec * specStrength;

    vec3 direct = (diffuse + specular);
    // final color
    vec3 color = ambient + (direct) * attenuation;
    FragColor = vec4(color, 1.0);
}
