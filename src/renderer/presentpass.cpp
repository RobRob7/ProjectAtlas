#include "presentpass.h"

#include "shader.h"

#include <glad/glad.h>

//--- PUBLIC ---//
PresentPass::~PresentPass()
{
	destroyGL();
} // end of destructor

void PresentPass::init()
{
	destroyGL();

	shader_ = std::make_unique<Shader>("presentpass/present.vert", "presentpass/present.frag");

	glCreateVertexArrays(1, &fsVao_);
} // end of init()

void PresentPass::resize(int w, int h)
{
	if (w <= 0 || h <= 0) return;
	if (w == width_ && h == height_) return;

	width_ = w;
	height_ = h;
} // end of resize()

void PresentPass::render(uint32_t sceneColorTex)
{
	if (!shader_ || !sceneColorTex || width_ <= 0 || height_ <= 0) 
		return;

	const GLboolean prevDepth = glIsEnabled(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width_, height_);
	glDisable(GL_DEPTH_TEST);

	shader_->use();
	shader_->setInt("u_sceneColorTex", 0);

	glBindTextureUnit(0, sceneColorTex);

	glBindVertexArray(fsVao_);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(0);

	if (prevDepth) glEnable(GL_DEPTH_TEST);
} // end of render()


//--- PRIVATE ---//
void PresentPass::destroyGL()
{
	if (fsVao_)
	{
		glDeleteVertexArrays(1, &fsVao_);
		fsVao_ = 0;
	}
} // end of destroyGL()