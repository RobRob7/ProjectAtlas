#version 460 core

layout (location = 0) flat in uvec2 Tile;
layout (location = 1) in vec2 TileUV;
layout (location = 2) in vec3 FragWorldPos;
layout (location = 3) in vec3 Normal;

layout (std140, set = 0, binding = 0) uniform UBO
{
    // vert
    vec3 u_chunkOrigin;
    float _pad0;

    mat4 u_view;
    mat4 u_proj;

    vec4 u_clipPlane;

    // frag
    vec3 u_viewPos;
    float _pad1;

    vec3 u_lightPos;
    float _pad2;

    vec3 u_lightColor;
    float u_ambientStrength;

    vec2 u_screenSize;
    int u_useSSAO;
    int _pad3;
};

layout (binding = 1) uniform sampler2D u_atlasTex;
layout (binding = 2) uniform sampler2D u_ssaoRaw;

layout(location = 0) out vec4 FragColor;

const float ATLAS_COLS = 32.0;
const float ATLAS_ROWS = 32.0;

const float INNER_TILE_SIZE = 16.0;
const float PADDED_TILE_SIZE = 32.0;
const float PADDING = 8.0;
const float INSET = 0.0;

vec2 atlasUV(uvec2 tile, vec2 local01)
{
    vec2 atlasPixelSize = vec2(
        ATLAS_COLS * PADDED_TILE_SIZE,
        ATLAS_ROWS * PADDED_TILE_SIZE
    );

    vec2 cellOriginPx = vec2(tile) * PADDED_TILE_SIZE;
    vec2 innerOriginPx = cellOriginPx + vec2(PADDING);

    vec2 innerSpan = vec2(INNER_TILE_SIZE - 1.0 - 2.0 * INSET);

    vec2 innerUVPx = innerOriginPx
                   + vec2(0.5 + INSET)
                   + local01 * innerSpan;

    return innerUVPx / atlasPixelSize;
} // end of atlasUV()

void main()
{
    // get correct UV (due to greedy meshing)
    vec2 tiled = TileUV;
    vec2 local = fract(tiled);

    vec2 uv = atlasUV(Tile, local);

    vec2 atlasPixelSize = vec2(
        ATLAS_COLS * PADDED_TILE_SIZE,
        ATLAS_ROWS * PADDED_TILE_SIZE
    );

    vec2 innerSpan = vec2(INNER_TILE_SIZE - 1.0 - 2.0 * INSET);
    vec2 uvScale = innerSpan / atlasPixelSize;

    vec2 dudx = dFdx(tiled) * uvScale;
    vec2 dudy = dFdy(tiled) * uvScale;

    vec4 texColor = textureGrad(u_atlasTex, uv, dudx, dudy);

    // allow textures to be see-through
    // (like tree canopy texture)
    if (texColor.a < 0.1)
    {
        discard;
    }

    // SSAO
    float ao = 1.0;
    if (u_useSSAO != 0)
    {
        vec2 ssUV = gl_FragCoord.xy / u_screenSize;
        ao = texture(u_ssaoRaw, ssUV).r;
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
