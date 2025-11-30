#include "cubemap.h"

//--- PUBLIC ---//
CubeMap::CubeMap(const std::vector<std::string>& textures)
	: shader_("/cubemap/cubemap.vert", "/cubemap/cubemap.frag"), texture_(textures)
{
	// get cubemap texture ID
	cubemapTexture_ = texture_.m_ID;

	// VAO + VBO
	glGenVertexArrays(1, &vao_);
	glGenBuffers(1, &vbo_);
	glBindVertexArray(vao_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SkyboxVertices), &SkyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	shader_.use();
	shader_.setInt("skybox", 0);
} // end of constructor

// destructor
CubeMap::~CubeMap()
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

} // end of destructor

// render cubemap
void CubeMap::render(glm::mat4& view, glm::mat4& projection) const
{
	glDepthFunc(GL_LEQUAL);
	shader_.use();
	shader_.setMat4("view", view);
	shader_.setMat4("projection", projection);
	glBindVertexArray(vao_);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture_);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
} // end of render()