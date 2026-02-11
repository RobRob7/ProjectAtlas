#ifndef CROSSHAIR_H
#define CROSSHAIR_H

class Shader;

#include <cstdint>
#include <memory>

class Crosshair
{
public:
	Crosshair(float size = 0.004f);
	~Crosshair();

	void init();
	void render();

private:
	void destroyGL();
private:
	std::unique_ptr<Shader> crosshairShader_;
	uint32_t vao_{};
	uint32_t vbo_{};
	const float size_{};
};

#endif
