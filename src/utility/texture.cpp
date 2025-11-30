#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

//--- PUBLIC ---//
Texture::Texture(const std::string& filePath)
	: filePath_(filePath)
{
} // end of constructor

void Texture::setupTexture()
{
	// texture path
	std::string pathToTexture = std::string(RESOURCES_PATH) + "texture/" + filePath_;

#ifdef _DEBUG
std::cout << "Loading Texture: " << pathToTexture << "\n";
#endif

	// flip image vertically
	stbi_set_flip_vertically_on_load(true);

	// load texture
	int w, h, nrChannels;
	unsigned char* data = stbi_load(pathToTexture.c_str(), &w, &h, &nrChannels, 0);

	// creation failure check
	if (!data)
	{
		std::cerr << "Failed to load texture!\n";
	}

	// set texture parameters
	width_ = w;
	height_ = h;
	colorChannels_ = nrChannels;

	// generate texture OpenGL
	glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
	glTextureStorage2D(m_ID, 1, GL_RGBA8, width_, height_);
	glTextureSubImage2D(m_ID, 0, 0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glTextureParameteri(m_ID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_ID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_ID, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(m_ID, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// free image
	stbi_image_free(data);
} // end of setupTexture()
