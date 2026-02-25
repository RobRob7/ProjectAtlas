#include "fog_pass.h"

#include "render_settings.h"

#include "texture_bindings.h"

#include "shader.h"

#include <glad/glad.h>

//--- PUBLIC ---//
FogPass::FogPass() = default;

FogPass::~FogPass()
{
	destroyGL();
} // end of destructor

void FogPass::init(RenderSettings& rs)
{
	destroyGL();

	shader_ = std::make_unique<Shader>("fogpass/fog.vert", "fogpass/fog.frag");

	ubo_.init<sizeof(FogPassUBO)>();

	rs.fogSettings.color = fogColor_;
	rs.fogSettings.start = fogStart_;
	rs.fogSettings.end = fogEnd_;

	glCreateVertexArrays(1, &fsVao_);
} // end of init()

void FogPass::resize(int w, int h)
{
	if (w <= 0 || h <= 0) return;
	if (w == width_ && h == height_) return;

	width_ = w;
	height_ = h;
} // end of resize()

void FogPass::render(uint32_t sceneColorTex, uint32_t sceneDepthTex, 
	float nearPlane, float farPlane, float ambStr)
{
	if (!shader_ || !sceneColorTex || !sceneDepthTex || fsVao_ == 0)
		return;

	const GLboolean prevDepth = glIsEnabled(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width_, height_);
	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(fsVao_);
	
	shader_->use();
	fogPassUBO_.u_near = nearPlane;
	fogPassUBO_.u_far = farPlane;
	fogPassUBO_.u_fogColor = fogColor_;
	fogPassUBO_.u_fogStart = fogStart_;
	fogPassUBO_.u_fogEnd = fogEnd_;
	fogPassUBO_.u_ambStr = ambStr;
	ubo_.update(&fogPassUBO_, sizeof(fogPassUBO_));

	glBindTextureUnit(TO_API_FORM(TextureBinding::ForwardColorTex), sceneColorTex);
	glBindTextureUnit(TO_API_FORM(TextureBinding::ForwardDepthTex), sceneDepthTex);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	if (prevDepth) glEnable(GL_DEPTH_TEST);
} // end of render()

void FogPass::setFogColor(glm::vec3 v)
{
	fogColor_ = v;
} // end of setFogColor()

void FogPass::setFogStart(float v)
{
	fogStart_ = v;
} // end of setFogStart()

void FogPass::setFogEnd(float v)
{
	fogEnd_ = v;
} // end of setFogEnd()


//--- PRIVATE ---//
void FogPass::destroyGL()
{
	if (fsVao_)
	{
		glDeleteVertexArrays(1, &fsVao_);
		fsVao_ = 0;
	}
} // end of destroyGL()