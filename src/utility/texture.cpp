#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

//--- PUBLIC ---//
Texture::Texture(const std::string& filePath, const bool needToFlip)
	: filePath_(filePath)
{
	// flip image vertically
	stbi_set_flip_vertically_on_load(needToFlip);
} // end of constructor

Texture::Texture(const std::vector<std::string>& textures, const bool needToFlip)
{
	// flip image vertically
	stbi_set_flip_vertically_on_load(needToFlip);

	// generate texture
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	// load texture
	int w, h, nrChannels;

	for (size_t i = 0; i < textures.size(); ++i)
	{
		std::string path = RESOURCES_PATH + textures[i];
		unsigned char* data = stbi_load(path.c_str(), &w, &h, &nrChannels, 0);
		width_ = w;
		height_ = h;
		colorChannels_ = nrChannels;
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width_, height_, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			// free image
			stbi_image_free(data);
		}
		else
		{
			std::cerr << "Cubemap texture failed to load at path: " << textures[i] << "\n";
			// free image
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// set texture ID
	m_ID = texture;
} // end of cubemap texture constructor

Texture::~Texture()
{
	if (m_ID)
	{
		glDeleteTextures(1, &m_ID);
	}
} // end of destructor

void Texture::setupTexture()
{
	// texture path
	std::string pathToTexture = std::string(RESOURCES_PATH) + "texture/" + filePath_;

#ifdef _DEBUG
	std::cout << "Loading Texture: " << pathToTexture << "\n";
#endif

	// load texture
	int w, h, nrChannels;
	unsigned char* data = stbi_load(pathToTexture.c_str(), &w, &h, &nrChannels, 0);

	// creation failure check
	if (!data)
	{
		std::cerr << "Failed to load texture!\n";
		return;
	}

#ifdef _DEBUG
	std::cout << "TEXTURE LOADED" << "\n";
#endif

	// set texture parameters
	width_ = w;
	height_ = h;
	colorChannels_ = nrChannels;

	// setup depending on color channels
	GLenum format = GL_RGBA;
	GLenum internalFormat = GL_RGBA8;

	if (colorChannels_ == 1)
	{
		format =		 GL_RED;
		internalFormat = GL_R8;
	}
	else if (colorChannels_ == 3)
	{
		format = GL_RGB;
		internalFormat = GL_RGB8;
	}
	else if (colorChannels_ == 4)
	{
		format = GL_RGBA;
		internalFormat = GL_RGBA8;
	}

	// generate texture OpenGL
	glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
	glTextureStorage2D(m_ID, 1, internalFormat, width_, height_);
	glTextureSubImage2D(m_ID, 0, 0, 0, width_, height_, format, GL_UNSIGNED_BYTE, data);

	glTextureParameteri(m_ID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_ID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(m_ID, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(m_ID, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// free image
	stbi_image_free(data);
} // end of setupTexture()
