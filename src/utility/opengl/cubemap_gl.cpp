#include "cubemap_gl.h"

#include "texture_bindings.h"

#include "texture.h"
#include "shader.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <cassert>

using namespace Cubemap_Constants;

//--- PUBLIC ---//
CubemapGL::CubemapGL(const std::array<std::string_view, 6>& textures)
	: faces_(textures)
{
} // end of constructor

// destructor
CubemapGL::~CubemapGL()
{
	destroyGL();
} // end of destructor

void CubemapGL::init()
{
	destroyGL();

	shader_ = std::make_unique<Shader>("cubemap/cubemap.vert", "cubemap/cubemap.frag");
	texture_ = std::make_unique<Texture>(faces_);

	// VAO + VBO
	glCreateVertexArrays(1, &vao_);
	glCreateBuffers(1, &vbo_);

	// upload vertex data
	glNamedBufferData(vbo_, SKYBOX_VERTICES.size() * sizeof(float), SKYBOX_VERTICES.data(), GL_STATIC_DRAW);

	// attach vbo to vao
	glVertexArrayVertexBuffer(vao_, 0, vbo_, 0, 3 * sizeof(float));

	// pos attribute
	glEnableVertexArrayAttrib(vao_, 0);
	glVertexArrayAttribFormat(vao_, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vao_, 0, 0);

	// UBO
	ubo_.init<sizeof(CubemapUBO)>();
} // end of init()

// render CubemapGL
void CubemapGL::render(const RenderContext& ctx, const glm::mat4& view, const glm::mat4& projection, const float time)
{
	assert(ctx.backend == RenderContext::Backend::OpenGL && "Must be OpenGL render context!");

	if (!shader_ || !texture_ || vao_ == 0)
		return;

	// remove translation from camera view
	glm::mat4 viewStrippedTranslation = glm::mat4(glm::mat3(view));

	if (time > 0.0f)
	{
		float speed = 0.005f;

		glm::mat4 skyRot = glm::rotate(glm::mat4(1.0f),
			time * speed,
			glm::vec3(0.0f, 1.0f, 0.0f));
		viewStrippedTranslation = viewStrippedTranslation * glm::mat4(glm::mat3(skyRot));
	}

	GLint prevFunc;
	glGetIntegerv(GL_DEPTH_FUNC, &prevFunc);
	GLboolean prevMask;
	glGetBooleanv(GL_DEPTH_WRITEMASK, &prevMask);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);

	shader_->use();
	CubemapUBO cubemapUBO{};
	cubemapUBO.view = viewStrippedTranslation;
	cubemapUBO.proj = projection;
	ubo_.update(&cubemapUBO, sizeof(CubemapUBO));
	glBindTextureUnit(TO_API_FORM(TextureBinding::CubemapTex), texture_->ID());

	glBindVertexArray(vao_);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	glDepthMask(prevMask);
	glDepthFunc(prevFunc);
} // end of render()


//--- PRIVATE ---//
void CubemapGL::destroyGL()
{
	if (vao_)
	{
		glDeleteVertexArrays(1, &vao_);
		vao_ = 0;
	}
	if (vbo_)
	{
		glDeleteBuffers(1, &vbo_);
		vbo_ = 0;
	}
} // end of destroyGL()