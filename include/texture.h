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
	uint32_t m_ID{};
public:
	Texture() = default;
	Texture(const std::string& filePath, const bool needToFlip = false);
	// constructor for cubemap texture
	Texture(const std::vector<std::string>& textures, const bool needToFlip = false);
	~Texture();

	// disallow copy
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	// move constructor
	Texture(Texture&& other) noexcept;
	// move assignment operator
	Texture& operator=(Texture&& other) noexcept;

private:
	std::string filePath_;
	// texture width
	int32_t width_{};
	// texture height
	int32_t height_{};
	// texture num of color channels
	int32_t colorChannels_{};
};

#endif
