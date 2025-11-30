#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

class Texture
{
public:
	// texture ID
	uint32_t m_ID = 0;
public:
	Texture(const std::string& filePath, const bool needToFlip = false);
	// constructor for cubemap texture
	Texture(const std::vector<std::string>& textures, const bool needToFlip = false);

	void setupTexture();

private:
	std::string filePath_;
	// texture width
	int32_t width_ = 0;
	// texture height
	int32_t height_ = 0;
	// texture num of color channels
	int32_t colorChannels_ = 0;
};

#endif
