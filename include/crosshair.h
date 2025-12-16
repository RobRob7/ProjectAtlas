#ifndef CROSSHAIR_H
#define CROSSHAIR_H

#include "shader.h"

#include <glad/glad.h>

#include <cstdint>
#include <optional>

class Crosshair
{
public:
	Crosshair(float size = 0.004f);
	~Crosshair();

	void init();
	void render();

private:
	std::optional<Shader> crosshairShader_;
	uint32_t vao_{};
	uint32_t vbo_{};
	const float size_{};
};

#endif
