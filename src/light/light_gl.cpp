#include "light_gl.h"

#include "shader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

#include <algorithm>
#include <cstddef>

//struct VertexLight
//{
//	glm::vec3 pos;
//	glm::vec3 normal;
//	glm::vec2 uv;
//};
//
//// pos, normals, texcoords
//const std::array<float, 288> CUBE_VERTICES = {
//	// =========================
//	// Back face (Z-)
//	// =========================
//	-0.5f,-0.5f,-0.5f,   0.0f,0.0f,-1.0f,   0.0f,0.0f,
//	 0.5f,-0.5f,-0.5f,   0.0f,0.0f,-1.0f,   1.0f,0.0f,
//	 0.5f, 0.5f,-0.5f,   0.0f,0.0f,-1.0f,   1.0f,1.0f,
//	 0.5f, 0.5f,-0.5f,   0.0f,0.0f,-1.0f,   1.0f,1.0f,
//	-0.5f, 0.5f,-0.5f,   0.0f,0.0f,-1.0f,   0.0f,1.0f,
//	-0.5f,-0.5f,-0.5f,   0.0f,0.0f,-1.0f,   0.0f,0.0f,
//
//	// =========================
//	// Front face (Z+)
//	// =========================
//	-0.5f,-0.5f, 0.5f,   0.0f,0.0f, 1.0f,   0.0f,0.0f,
//	 0.5f,-0.5f, 0.5f,   0.0f,0.0f, 1.0f,   1.0f,0.0f,
//	 0.5f, 0.5f, 0.5f,   0.0f,0.0f, 1.0f,   1.0f,1.0f,
//	 0.5f, 0.5f, 0.5f,   0.0f,0.0f, 1.0f,   1.0f,1.0f,
//	-0.5f, 0.5f, 0.5f,   0.0f,0.0f, 1.0f,   0.0f,1.0f,
//	-0.5f,-0.5f, 0.5f,   0.0f,0.0f, 1.0f,   0.0f,0.0f,
//
//	// =========================
//	// Left face (X-)
//	// =========================
//	-0.5f, 0.5f, 0.5f,  -1.0f,0.0f,0.0f,   1.0f,0.0f,
//	-0.5f, 0.5f,-0.5f,  -1.0f,0.0f,0.0f,   1.0f,1.0f,
//	-0.5f,-0.5f,-0.5f,  -1.0f,0.0f,0.0f,   0.0f,1.0f,
//	-0.5f,-0.5f,-0.5f,  -1.0f,0.0f,0.0f,   0.0f,1.0f,
//	-0.5f,-0.5f, 0.5f,  -1.0f,0.0f,0.0f,   0.0f,0.0f,
//	-0.5f, 0.5f, 0.5f,  -1.0f,0.0f,0.0f,   1.0f,0.0f,
//
//	// =========================
//	// Right face (X+)
//	// =========================
//	 0.5f, 0.5f, 0.5f,   1.0f,0.0f,0.0f,   1.0f,0.0f,
//	 0.5f, 0.5f,-0.5f,   1.0f,0.0f,0.0f,   1.0f,1.0f,
//	 0.5f,-0.5f,-0.5f,   1.0f,0.0f,0.0f,   0.0f,1.0f,
//	 0.5f,-0.5f,-0.5f,   1.0f,0.0f,0.0f,   0.0f,1.0f,
//	 0.5f,-0.5f, 0.5f,   1.0f,0.0f,0.0f,   0.0f,0.0f,
//	 0.5f, 0.5f, 0.5f,   1.0f,0.0f,0.0f,   1.0f,0.0f,
//
//	 // =========================
//	 // Bottom face (Y-)
//	 // =========================
//	 -0.5f,-0.5f,-0.5f,   0.0f,-1.0f,0.0f,   0.0f,1.0f,
//	  0.5f,-0.5f,-0.5f,   0.0f,-1.0f,0.0f,   1.0f,1.0f,
//	  0.5f,-0.5f, 0.5f,   0.0f,-1.0f,0.0f,   1.0f,0.0f,
//	  0.5f,-0.5f, 0.5f,   0.0f,-1.0f,0.0f,   1.0f,0.0f,
//	 -0.5f,-0.5f, 0.5f,   0.0f,-1.0f,0.0f,   0.0f,0.0f,
//	 -0.5f,-0.5f,-0.5f,   0.0f,-1.0f,0.0f,   0.0f,1.0f,
//
//	 // =========================
//	 // Top face (Y+)
//	 // =========================
//	 -0.5f, 0.5f,-0.5f,   0.0f,1.0f,0.0f,   0.0f,1.0f,
//	  0.5f, 0.5f,-0.5f,   0.0f,1.0f,0.0f,   1.0f,1.0f,
//	  0.5f, 0.5f, 0.5f,   0.0f,1.0f,0.0f,   1.0f,0.0f,
//	  0.5f, 0.5f, 0.5f,   0.0f,1.0f,0.0f,   1.0f,0.0f,
//	 -0.5f, 0.5f, 0.5f,   0.0f,1.0f,0.0f,   0.0f,0.0f,
//	 -0.5f, 0.5f,-0.5f,   0.0f,1.0f,0.0f,   0.0f,1.0f
//};

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

	//// normal
	//glEnableVertexArrayAttrib(vao_, 1);
	//glVertexArrayAttribFormat(vao_, 1, 3, GL_FLOAT, GL_FALSE, offsetof(VertexLight, normal));
	//glVertexArrayAttribBinding(vao_, 1, 0);

	//// uv
	//glEnableVertexArrayAttrib(vao_, 2);
	//glVertexArrayAttribFormat(vao_, 2, 2, GL_FLOAT, GL_FALSE, offsetof(VertexLight, uv));
	//glVertexArrayAttribBinding(vao_, 2, 0);
} // end of init()

void LightGL::render(const glm::mat4& view, const glm::mat4& proj)
{
	if (!shader_ || vao_ == 0) 
		return;

	shader_->use();
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position_);
	
	shader_->setVec3("u_color", color_);
	shader_->setMat4("u_model", model);
	shader_->setMat4("u_view", view);
	shader_->setMat4("u_proj", proj);

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
} // end of destroyGL()