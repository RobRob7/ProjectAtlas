#include "texture.h"

#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <iostream>

//--- PUBLIC ---//
Texture::Texture(const std::string& filePath, const bool needToFlip)
	: filePath_(filePath)
{
	// flip image vertically
	stbi_set_flip_vertically_on_load(needToFlip);

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
		std::cerr << "Failed to load texture: " << pathToTexture
			<< " reason: " << stbi_failure_reason() << "\n";
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
		format = GL_RED;
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
	glCreateTextures(GL_TEXTURE_2D, 1, &id_);
	int levels = 1 + static_cast<int>(std::floor(std::log2(std::max(width_, height_))));
	glTextureStorage2D(id_, levels, internalFormat, width_, height_);
	glTextureSubImage2D(id_, 0, 0, 0, width_, height_, format, GL_UNSIGNED_BYTE, data);
	glGenerateTextureMipmap(id_);

	glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(id_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(id_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// free image
	stbi_image_free(data);
} // end of constructor

Texture::Texture(const std::array<std::string_view, 6>& textures, const bool needToFlip)
{
	stbi_set_flip_vertically_on_load(needToFlip);

	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &id_);

	int w = 0, h = 0, n = 0;

	// load first face to determine size/format
	{
		std::string path = std::string(RESOURCES_PATH) + std::string(textures[0]);
		unsigned char* data = stbi_load(path.c_str(), &w, &h, &n, 0);
		if (!data)
		{
			std::cerr << "Cubemap face 0 failed to load: " << textures[0] << "\n";
			return;
		}

		width_ = w;
		height_ = h;
		colorChannels_ = n;

		stbi_image_free(data);
	}

	GLenum srcFormat = (colorChannels_ == 4) ? GL_RGBA : GL_RGB;
	GLenum internalFormat = (colorChannels_ == 4) ? GL_RGBA8 : GL_RGB8;

	glTextureStorage2D(id_, 1, internalFormat, width_, height_);

	for (int i = 0; i < 6; ++i)
	{
		std::string path = std::string(RESOURCES_PATH) + std::string(textures[i]);

		int fw = 0, fh = 0, fn = 0;
		unsigned char* data = stbi_load(path.c_str(), &fw, &fh, &fn, 0);
		if (!data)
		{
			std::cerr << "Cubemap texture failed to load: " << textures[i] << "\n";
			continue;
		}

		if (fw != width_ || fh != height_ || fn != colorChannels_)
		{
			std::cerr << "Cubemap face mismatch (size/format) at: " << textures[i] << "\n";
			stbi_image_free(data);
			continue;
		}

		// avoid row alignment issues for non-4-byte row sizes
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTextureSubImage3D(
			id_,
			0,
			0, 0, i,
			width_, height_, 1,
			srcFormat,
			GL_UNSIGNED_BYTE,
			data
		);

		stbi_image_free(data);
	}

	glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(id_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(id_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(id_, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// restore default alignment
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
} // end of cubemap texture constructor

Texture::~Texture()
{
	if (id_)
	{
		glDeleteTextures(1, &id_);
		id_ = 0;
	}
} // end of destructor

Texture::Texture(Texture&& other) noexcept
	: id_(other.id_),
	  filePath_(std::move(other.filePath_)),
	  width_(other.width_),
      height_(other.height_),
	  colorChannels_(other.colorChannels_)
{
	other.id_ = 0;
	other.width_ = 0;
	other.height_ = 0;
	other.colorChannels_ = 0;
} // end of move constructor

Texture& Texture::operator=(Texture&& other) noexcept
{
	if (this != &other)
	{
		if (id_ != 0)
		{
			glDeleteTextures(1, &id_);
		}

		id_ = other.id_;
		filePath_ = std::move(other.filePath_);
		width_ = other.width_;
		height_ = other.height_;
		colorChannels_ = other.colorChannels_;

		other.id_ = 0;
		other.width_ = 0;
		other.height_ = 0;
		other.colorChannels_ = 0;
	}
	return *this;
} // end of move assignment

uint32_t Texture::ID() const
{
	return id_;
} // end of getID

void Texture::setWrapRepeat() const
{
	glTextureParameteri(id_, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(id_, GL_TEXTURE_WRAP_T, GL_REPEAT);
} // end of setWrapRepeat()

void Texture::setNoMipmapsLinear() const
{
	// disable mipmaps
	glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// prevent mipmap sampling
	glTextureParameteri(id_, GL_TEXTURE_BASE_LEVEL, 0);
	glTextureParameteri(id_, GL_TEXTURE_MAX_LEVEL, 0);
} // end of setNoMipmapsLinear()
