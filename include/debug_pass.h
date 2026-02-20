#ifndef DEBUGPASS_H
#define DEBUGPASS_H

#include <memory>
#include <cstdint>

class Shader;

class DebugPass
{
public:
	DebugPass();
	~DebugPass();

	void init();
	void destroyGL();
	void render(uint32_t normalTex, uint32_t depthTex, float nearPlane, float farPlane, int mode);

private:
	uint32_t vao_{};
	std::unique_ptr<Shader> debugShader_;
};

#endif
