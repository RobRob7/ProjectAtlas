#ifndef TEXTURE_H
#define TEXTURE_H

#include <cstdint>
#include <string>
#include <string_view>
#include <array>

class Texture
{
public:
	Texture(const std::string& filePath, const bool needToFlip = false);
	// constructor for cubemap texture
	Texture(const std::array<std::string_view, 6>& textures, const bool needToFlip = false);
	~Texture();

	// disallow copy
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	// move constructor
	Texture(Texture&& other) noexcept;
	// move assignment operator
	Texture& operator=(Texture&& other) noexcept;

	void setWrapRepeat();
	uint32_t ID() const;

private:
	uint32_t id_{};
	std::string filePath_;
	// texture width
	int32_t width_{};
	// texture height
	int32_t height_{};
	// texture num of color channels
	int32_t colorChannels_{};
};

#endif
