#include "chunk_pass_gl.h"

#include "chunk_draw_list.h"

#include "i_chunk_mesh_gpu.h"

#include "shader.h"
#include "texture.h"
#include "camera.h"
#include "light_gl.h"

#include "render_inputs.h"
#include "chunk_manager.h"
#include "renderer_gl.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

//--- PUBLIC ---//
ChunkPassGL::~ChunkPassGL() = default;

void ChunkPassGL::init()
{
	opaqueShader_ = std::make_unique<Shader>("chunk/chunk.vert", "chunk/chunk.frag");
    waterShader_ = std::make_unique<Shader>("water/water.vert", "water/water.frag");
	atlas_ = std::make_unique<Texture>("blocks.png", true);

    uboOpaque_.init<sizeof(ChunkOpaqueUBO)>();
    uboWater_.init<sizeof(ChunkWaterUBO)>();
} // end of init()

void ChunkPassGL::updateShader(const RenderInputs& in, const RenderSettings& rs, const int w, const int h)
{
    // update uniforms of opaque shader
    opaqueShader_->use();
    chunkOpaqueUBO_.u_ambientStrength = in.world->getAmbientStrength();
    chunkOpaqueUBO_.u_viewPos = in.camera->getCameraPosition();
    chunkOpaqueUBO_.u_lightPos = in.light->getPosition();
    chunkOpaqueUBO_.u_lightColor = in.light->getColor();

    // ssao
    chunkOpaqueUBO_.u_screenSize = glm::vec2{ w, h };
    chunkOpaqueUBO_.u_useSSAO = rs.useSSAO ? 1 : 0;
    opaqueShader_->setInt("u_ssao", 3);
    uboOpaque_.update(&chunkOpaqueUBO_, sizeof(chunkOpaqueUBO_));

    // update uniforms of water shader
    waterShader_->use();

    chunkWaterUBO_.u_time = in.time;
    chunkWaterUBO_.u_near = in.camera->getNearPlane();
    chunkWaterUBO_.u_far = in.camera->getFarPlane();
    chunkWaterUBO_.u_screenSize = glm::vec2{ w, h };
    chunkWaterUBO_.u_viewPos = in.camera->getCameraPosition();
    chunkWaterUBO_.u_lightPos = in.light->getPosition();
    chunkWaterUBO_.u_lightColor = in.light->getColor();
    chunkWaterUBO_.u_ambientStrength = in.world->getAmbientStrength();

    waterShader_->setInt("u_reflectionTex", 4);
    waterShader_->setInt("u_refractionTex", 5);
    waterShader_->setInt("u_refractionDepthTex", 6);
    waterShader_->setInt("u_dudvTex", 7);
    waterShader_->setInt("u_normalTex", 8);
    uboWater_.update(&chunkWaterUBO_, sizeof(chunkWaterUBO_));
} // end of updateShader()

void ChunkPassGL::renderOpaque(
	const RenderInputs& in,
	const glm::mat4& view,
	const glm::mat4& proj,
	int width, int height)
{
    ChunkDrawList list;
    in.world->buildOpaqueDrawList(view, proj, list);

    opaqueShader_->use();
    chunkOpaqueUBO_.u_view = view;
    chunkOpaqueUBO_.u_proj = proj;
    chunkOpaqueUBO_.u_screenSize = glm::vec2{ width, height };

    glBindTextureUnit(0, atlas_->ID());
    opaqueShader_->setInt("u_atlas", 0);

    DrawContext ctx{};
    for (const auto& item : list.items)
    {
        chunkOpaqueUBO_.u_chunkOrigin = item.chunkOrigin;
        uboOpaque_.update(&chunkOpaqueUBO_, sizeof(chunkOpaqueUBO_));
        item.gpu->drawOpaque(ctx);
    }
} // end of renderOpaque()

void ChunkPassGL::renderOpaque(
    Shader& shader,
    const RenderInputs& in,
    const glm::mat4& view,
    const glm::mat4& proj,
    int width, int height)
{
    ChunkDrawList list;
    in.world->buildOpaqueDrawList(view, proj, list);

    shader.use();
    shader.setMat4("u_view", view);
    shader.setMat4("u_proj", proj);

    DrawContext ctx{};
    for (const auto& item : list.items)
    {
        shader.setVec3("u_chunkOrigin", item.chunkOrigin);
        item.gpu->drawOpaque(ctx);
    }
} // end of renderOpaque()

void ChunkPassGL::renderWater(
    const RenderInputs& in,
    const glm::mat4& view,
    const glm::mat4& proj,
    int width, int height)
{
    ChunkDrawList list;
    in.world->buildWaterDrawList(view, proj, list);

    waterShader_->use();
    chunkWaterUBO_.u_view = view;
    chunkWaterUBO_.u_proj = proj;
    chunkWaterUBO_.u_screenSize = glm::vec2{ width, height };

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glEnable(GL_DEPTH_TEST);

    DrawContext ctx{};
    for (const auto& item : list.items)
    {
        glm::mat4 model = glm::translate(
            glm::mat4(1.0f),
            item.chunkOrigin);
        chunkWaterUBO_.u_model = model;
        uboWater_.update(&chunkWaterUBO_, sizeof(chunkWaterUBO_));
        item.gpu->drawWater(ctx);
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
} // end of renderWater()