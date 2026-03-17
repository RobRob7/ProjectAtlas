#ifndef CROSSHAIR_H
#define CROSSHAIR_H

#include <cstdint>
#include <memory>

class Shader;

class Crosshair
{
public:
	Crosshair();
	~Crosshair();

	void init();
	void render();

private:
	void destroyGL();
private:
	std::unique_ptr<Shader> crosshairShader_;
	uint32_t vao_{};
	uint32_t vbo_{};
};

#endif
