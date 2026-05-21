#ifndef MESH_GL_H
#define MESH_GL_H

#include "shader.h"

#include <glm/glm.hpp>

#include <vector>
#include <cstdint>
#include <string>

struct MeshVertex 
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

struct Texture
{
	uint32_t id;
	std::string type;
	std::string path;
};

class MeshGL
{
public:
	MeshGL(
		const std::vector<MeshVertex>& vertices,
		const std::vector<uint32_t>& indices,
		const std::vector<Texture>& textures);
	~MeshGL();

	MeshGL(const MeshGL&) = delete;
	MeshGL& operator=(const MeshGL&) = delete;

	MeshGL(MeshGL&& other) noexcept;
	MeshGL& operator=(MeshGL&& other) noexcept;

	void render(Shader& shader);

	const std::vector<MeshVertex>& getVertices() const { return vertices_; }
	const std::vector<uint32_t>& getIndices() const { return indices_; }
	const std::vector<Texture>& getTextures() const { return textures_; }

private:
	void setupMesh();
private:
	uint32_t vao_{};
	uint32_t vbo_{};
	uint32_t ebo_{};

	std::vector<MeshVertex> vertices_;
	std::vector<uint32_t> indices_;
	std::vector<Texture> textures_;
};

#endif
