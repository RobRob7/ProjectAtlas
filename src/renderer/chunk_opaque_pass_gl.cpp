#include "chunk_opaque_pass_gl.h"

#include "chunk_draw_list.h"

#include "i_chunk_mesh_gpu.h"

#include "shader.h"
#include "texture.h"
#include "camera.h"
#include "light.h"

#include "render_inputs.h"
#include "chunk_manager.h"
#include "renderer.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

//--- PUBLIC ---//
ChunkOpaquePassGL::~ChunkOpaquePassGL() = default;

void ChunkOpaquePassGL::init()
{
	opaqueShader_ = std::make_unique<Shader>("chunk/chunk.vert", "chunk/chunk.frag");
    waterShader_ = std::make_unique<Shader>("water/water.vert", "water/water.frag");
	atlas_ = std::make_unique<Texture>("blocks.png", true);
} // end of init()

void ChunkOpaquePassGL::updateShader(const RenderInputs& in, const RenderSettings& rs, const int w, const int h)
{
    // update uniforms of opaque shader
    opaqueShader_->use();
    opaqueShader_->setFloat("u_ambientStrength", in.world->getAmbientStrength());
    opaqueShader_->setVec3("u_viewPos", in.camera->getCameraPosition());
    opaqueShader_->setVec3("u_lightPos", in.light->getPosition());
    opaqueShader_->setVec3("u_lightColor", in.light->getColor());

    // ssao
    opaqueShader_->setVec2("u_screenSize", glm::vec2{ w, h });
    opaqueShader_->setBool("u_useSSAO", rs.useSSAO);
    opaqueShader_->setInt("u_ssao", 3);

    // update uniforms of water shader
    waterShader_->use();
    waterShader_->setFloat("u_ambientStrength", in.world->getAmbientStrength());
    waterShader_->setVec3("u_viewPos", in.camera->getCameraPosition());
    waterShader_->setVec3("u_lightPos", in.light->getPosition());
    waterShader_->setVec3("u_lightColor", in.light->getColor());
    waterShader_->setFloat("u_near", in.camera->getNearPlane());
    waterShader_->setFloat("u_far", in.camera->getFarPlane());
    waterShader_->setVec2("u_screenSize", glm::vec2{ w, h });

    waterShader_->setInt("u_reflectionTex", 4);
    waterShader_->setInt("u_refractionTex", 5);
    waterShader_->setInt("u_refractionDepthTex", 6);
    waterShader_->setInt("u_dudvTex", 7);
    waterShader_->setInt("u_normalTex", 8);
    waterShader_->setFloat("u_time", in.time);
} // end of updateOpaqueShader()

void ChunkOpaquePassGL::renderOpaque(
	const RenderInputs& in,
	const glm::mat4& view,
	const glm::mat4& proj,
	int width, int height)
{
    ChunkDrawList list;
    in.world->buildOpaqueDrawList(view, proj, list);

    opaqueShader_->use();
    opaqueShader_->setMat4("u_view", view);
    opaqueShader_->setMat4("u_proj", proj);

    glBindTextureUnit(0, atlas_->ID());
    opaqueShader_->setInt("u_atlas", 0);
    //opaqueShader_->setFloat("u_ambientStrength", in.world->getAmbientStrength());
    //opaqueShader_->setVec3("u_viewPos", in.camera->getCameraPosition());
    //opaqueShader_->setVec3("u_lightPos", in.light->getPosition());
    //opaqueShader_->setVec3("u_lightColor", in.light->getColor());

    opaqueShader_->setVec2("u_screenSize", glm::vec2{ width, height });

    for (const auto& item : list.items)
    {
        opaqueShader_->setVec3("u_chunkOrigin", item.chunkOrigin);
        item.gpu->drawOpaque();
    }
} // end of renderOpaque()

void ChunkOpaquePassGL::renderOpaque(
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

    for (const auto& item : list.items)
    {
        shader.setVec3("u_chunkOrigin", item.chunkOrigin);
        item.gpu->drawOpaque();
    }
} // end of renderOpaque()

void ChunkOpaquePassGL::renderWater(
    const RenderInputs& in,
    const glm::mat4& view,
    const glm::mat4& proj,
    int width, int height)
{
    ChunkDrawList list;
    in.world->buildWaterDrawList(view, proj, list);

    waterShader_->use();
    waterShader_->setMat4("u_view", view);
    waterShader_->setMat4("u_proj", proj);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glEnable(GL_DEPTH_TEST);

    for (const auto& item : list.items)
    {
        glm::mat4 model = glm::translate(
            glm::mat4(1.0f),
            item.chunkOrigin);
        waterShader_->setMat4("u_model", model);
        item.gpu->drawWater();
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
} // end of renderWater()