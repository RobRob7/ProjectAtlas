#include "light_gl.h"

#include "shader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

#include <cstddef>
#include <cassert>

using namespace Light_Constants;

//--- PUBLIC ---//
LightGL::LightGL(const glm::vec3& pos, const glm::vec3& color)
	: position_(pos)
{
	setColor(color);
} // end of constructor

LightGL::~LightGL()
{
	destroyGL();
} // end of destructor

void LightGL::init()
{
	destroyGL();

	shader_ = std::make_unique<Shader>("light/light.vert", "light/light.frag");

	// vao + vbo
	glCreateVertexArrays(1, &vao_);
	glCreateBuffers(1, &vbo_);

	// upload data to buffer
	glNamedBufferData(vbo_, CUBE_VERTICES.size() * sizeof(float), CUBE_VERTICES.data(), GL_STATIC_DRAW);

	// attach buffers to vao
	glVertexArrayVertexBuffer(vao_, 0, vbo_, 0, sizeof(VertexLight));

	// position
	glEnableVertexArrayAttrib(vao_, 0);
	glVertexArrayAttribFormat(vao_, 0, 3, GL_FLOAT, GL_FALSE, offsetof(VertexLight, pos));
	glVertexArrayAttribBinding(vao_, 0, 0);

	// UBO
	ubo_.init<sizeof(LightUBO)>();
} // end of init()

void LightGL::render(const RenderContext& ctx, const glm::mat4& view, const glm::mat4& proj)
{
	assert(ctx.backend == Backend::OpenGL && "Must be OpenGL render context!");

	if (!shader_ || vao_ == 0)
		return;

	shader_->use();
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position_);

	LightUBO lightUBO{};
	lightUBO.model = model;
	lightUBO.view = view;
	lightUBO.proj = proj;
	lightUBO.color = glm::vec4(color_, 1.0f);
	ubo_.update(&lightUBO, sizeof(lightUBO));

	glBindVertexArray(vao_);
	glDrawArrays(GL_TRIANGLES, 0, 36);
} // end of render


//--- PRIVATE ---//
void LightGL::destroyGL()
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