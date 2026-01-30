#ifndef DEBUGPASS_H
#define DEBUGPASS_H

#include "shader.h"

#include <optional>
#include <cstdint>

class DebugPass
{
public:
	~DebugPass();

	void init();
	void destroyGL();
	void render(uint32_t normalTex, uint32_t depthTex, float nearPlane, float farPlane, int mode);

private:
	std::optional<Shader> debugShader_;
	uint32_t vao_;
};

#endif
