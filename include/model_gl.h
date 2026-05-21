#ifndef MODEL_GL_H
#define MODEL_GL_H

#include "mesh_gl.h"

#include "shader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <string>
#include <memory>

struct ModelUBO
{
	glm::mat4 u_model;
	glm::mat4 u_view;
	glm::mat4 u_proj;

	glm::vec4 u_viewPos;
	glm::vec4 u_lightDir;
	glm::vec4 u_lightColor;

	float u_ambientStrength;
	float pad0;
	float pad1;
	float pad2;
};

class UBOGL;

class ModelGL
{
public:
	ModelGL(const std::string& path);
	~ModelGL();

	void render(
		Shader& shader,
		UBOGL& ubo,
		const ModelUBO& uboData
	);

private:
	void loadModel(std::string path);

	void processNode(
		aiNode* node, 
		const aiScene* scene
	);

	MeshGL processMesh(
		aiMesh* mesh, 
		const aiScene* scene
	);

	std::vector<Texture> loadMaterialTextures(
		aiMaterial* mat, 
		aiTextureType type,
		std::string typeName
	);
private:
	std::vector<MeshGL> meshes_;
	std::string directory_;
	std::vector<Texture> texturesLoaded_;
};

#endif
