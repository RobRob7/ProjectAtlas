#include "model_gl.h"

#include "ubo_gl.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stb/stb_image.h>

#include <iostream>

//--- HELPERS ---//
static uint32_t TextureFromFile(
	const char* path, 
	const std::string& directory
)
{
	stbi_set_flip_vertically_on_load(true);

	std::string filename = std::string(path);
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
} // end of TextureFromFile()


//--- PUBLIC ---//
ModelGL::ModelGL(const std::string& path)
{
	loadModel(path);
} // end of constructor

ModelGL::~ModelGL() = default;

void ModelGL::render(
	Shader& shader,
	UBOGL& ubo,
	const ModelUBO& uboData
)
{
	shader.use();

	ubo.update(&uboData, sizeof(ModelUBO));
	ubo.bind();

	for (uint32_t i = 0; i < meshes_.size(); ++i)
	{
		meshes_[i].render(shader);
	} // end for


} // end of render()


//--- PRIVATE ---//
void ModelGL::loadModel(std::string path)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(
		path,
		aiProcess_Triangulate |
		aiProcess_FlipUVs
	);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}

	directory_ = path.substr(0, path.find_last_of('/'));

	processNode(scene->mRootNode, scene);
} // end of loadModel()

void ModelGL::processNode(
	aiNode* node,
	const aiScene* scene
)
{
	for (uint32_t i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes_.push_back(processMesh(mesh, scene));
	} // end for

	for (uint32_t i = 0; i < node->mNumChildren; ++i)
	{
		processNode(node->mChildren[i], scene);
	} // end for
} // end of processNode()

MeshGL ModelGL::processMesh(
	aiMesh* mesh,
	const aiScene* scene
)
{
	std::vector<MeshVertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<Texture> textures;

	// process vertices
	for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
	{
		MeshVertex vertex;
		glm::vec3 vector;

		// pos
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;
		// normals
		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.Normal = vector;
		// texture coords
		if (mesh->mTextureCoords[0])
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else
		{
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		}

		vertices.push_back(vertex);
	} // end for

	// process indices
	for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		for (uint32_t j = 0; j < face.mNumIndices; ++j)
		{
			indices.push_back(face.mIndices[j]);
		} // end for
	} // end for

	// process material
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseMaps = loadMaterialTextures(
			material,
			aiTextureType_DIFFUSE,
			"texture_diffuse"
		);
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		std::vector<Texture> specularMaps = loadMaterialTextures(
			material,
			aiTextureType_SPECULAR,
			"texture_specular"
		);
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	return MeshGL(vertices, indices, textures);
} // end of processMesh()

std::vector<Texture> ModelGL::loadMaterialTextures(
	aiMaterial* mat,
	aiTextureType type,
	std::string typeName
)
{
	std::vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		bool skip = false;
		for (unsigned int j = 0; j < texturesLoaded_.size(); j++)
		{
			if (std::strcmp(texturesLoaded_[j].path.data(), str.C_Str()) == 0)
			{
				textures.push_back(texturesLoaded_[j]);
				skip = true;
				break;
			}
		}
		if (!skip)
		{   // if texture hasn't been loaded already, load it
			Texture texture;
			texture.id = TextureFromFile(str.C_Str(), directory_);
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			texturesLoaded_.push_back(texture);
		}
	} // end for

	return textures;
} // end of loadMaterialTextures()