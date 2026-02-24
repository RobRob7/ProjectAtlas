#include "light_gl.h"

#include "ubo_bindings.h"

#include "shader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

#include <algorithm>
#include <cstddef>

struct LightUBO
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
	glm::vec4 color;
};

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

void LightGL::render(const glm::mat4& view, const glm::mat4& proj)
{
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

glm::vec3& LightGL::getPosition()
{
	return position_;
} // end of getPosition()

const glm::vec3& LightGL::getPosition() const
{
	return position_;
} // end of getPosition()

glm::vec3& LightGL::getColor()
{
	return color_;
} // end of getColor()

const glm::vec3& LightGL::getColor() const
{
	return color_;
} // end of getColor()

void LightGL::setPosition(const glm::vec3& pos)
{
	position_ = pos;
} // end of setPosition()

void LightGL::setColor(const glm::vec3& color)
{
	color_ = {
		std::clamp(color.x, MIN_COLOR, MAX_COLOR),
		std::clamp(color.y, MIN_COLOR, MAX_COLOR),
		std::clamp(color.z, MIN_COLOR, MAX_COLOR)
	};
} // end of setColor()


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
	//if (ubo_)
	//{
	//	glDeleteBuffers(1, &ubo_);
	//	ubo_ = 0;
	//}
} // end of destroyGL()