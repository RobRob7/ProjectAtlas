#ifndef CROSSHAIR_H
#define CROSSHAIR_H

#include "shader.h"

#include <cstdint>

class Crosshair
{
public:
	Crosshair() = default;
	Crosshair(const float size);
	~Crosshair();

	void render();

private:
	Shader crosshairShader_;
	uint32_t vao_;
	uint32_t vbo_;
	const float size_;
};

#endif
