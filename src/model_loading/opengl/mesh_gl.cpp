#include "mesh_gl.h"

#include <glad/glad.h>


//--- PUBLIC ---//
MeshGL::MeshGL(
	const std::vector<MeshVertex>& vertices,
	const std::vector<uint32_t>& indices,
	const std::vector<Texture>& textures)
{
	vertices_ = vertices;
	indices_ = indices;
	textures_ = textures;

	setupMesh();
} // end of constructor

MeshGL::~MeshGL()
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

	if (ebo_)
	{
		glDeleteBuffers(1, &ebo_);
		ebo_ = 0;
	}
} // end of destructor

MeshGL::MeshGL(MeshGL&& other) noexcept
	: vao_(other.vao_),
	vbo_(other.vbo_),
	ebo_(other.ebo_),
	vertices_(std::move(other.vertices_)),
	indices_(std::move(other.indices_)),
	textures_(std::move(other.textures_))
{
	other.vao_ = 0;
	other.vbo_ = 0;
	other.ebo_ = 0;
}

MeshGL& MeshGL::operator=(MeshGL&& other) noexcept
{
	if (this != &other)
	{
		this->~MeshGL();

		vao_ = other.vao_;
		vbo_ = other.vbo_;
		ebo_ = other.ebo_;

		vertices_ = std::move(other.vertices_);
		indices_ = std::move(other.indices_);
		textures_ = std::move(other.textures_);

		other.vao_ = 0;
		other.vbo_ = 0;
		other.ebo_ = 0;
	}
	return *this;
}

void MeshGL::render(Shader& shader)
{
	uint32_t diffuseNr = 1;
	uint32_t specularNr = 1;

	for (uint32_t i = 0; i < textures_.size(); ++i)
	{
		glBindTextureUnit(i, textures_[i].id);

		std::string number;
		std::string name = textures_[i].type;

		if (name == "texture_diffuse")
		{
			number = std::to_string(diffuseNr++);
		}
		else if (name == "texture_specular")
		{
			number = std::to_string(specularNr++);
		}

		shader.setInt(("material." + name + number).c_str(), i);
	} // end for

	glBindVertexArray(vao_);
	glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
} // end of render()


//--- PRIVATE ---//
void MeshGL::setupMesh()
{
	glCreateVertexArrays(1, &vao_);
	glCreateBuffers(1, &vbo_);
	glCreateBuffers(1, &ebo_);

	glNamedBufferStorage(
		vbo_, 
		vertices_.size() * sizeof(MeshVertex),
		vertices_.data(),
		0
	);
	glNamedBufferStorage(
		ebo_, 
		indices_.size() * sizeof(uint32_t), 
		indices_.data(),
		0
	);

	glVertexArrayVertexBuffer(
		vao_,
		0,
		vbo_,
		0,
		sizeof(MeshVertex)
	);
	glVertexArrayElementBuffer(
		vao_,
		ebo_
	);

	// position
	glEnableVertexArrayAttrib(vao_, 0);
	glVertexArrayAttribFormat(
		vao_,
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		0
	);
	glVertexArrayAttribBinding(vao_, 0, 0);

	// normals
	glEnableVertexArrayAttrib(vao_, 1);
	glVertexArrayAttribFormat(
		vao_,
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		offsetof(MeshVertex, Normal)
	);
	glVertexArrayAttribBinding(vao_, 1, 0);

	// texture coords
	glEnableVertexArrayAttrib(vao_, 2);
	glVertexArrayAttribFormat(
		vao_,
		2,
		2,
		GL_FLOAT,
		GL_FALSE,
		offsetof(MeshVertex, TexCoords)
	);
	glVertexArrayAttribBinding(vao_, 2, 0);
} // end of setupMesh()
