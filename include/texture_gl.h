#ifndef TEXTURE_GL_H
#define TEXTURE_GL_H

#include <cstdint>
#include <string>
#include <string_view>
#include <array>

class TextureGL
{
public:
	TextureGL(const std::string& filePath, const bool needToFlip = false);
	// constructor for cubemap texture
	TextureGL(const std::array<std::string_view, 6>& textures, const bool needToFlip = false);
	~TextureGL();

	// disallow copy
	TextureGL(const TextureGL&) = delete;
	TextureGL& operator=(const TextureGL&) = delete;

	// move constructor
	TextureGL(TextureGL&& other) noexcept;
	// move assignment operator
	TextureGL& operator=(TextureGL&& other) noexcept;

	uint32_t ID() const;
	void setWrapRepeat() const;
	void setNoMipmapsLinear() const;

	int32_t getWidth() const;
	int32_t getHeight() const;

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
