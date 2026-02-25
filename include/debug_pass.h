#ifndef DEBUG_PASS_H
#define DEBUG_PASS_H

#include "ubo_gl.h"

#include <memory>
#include <cstdint>

struct DebugPassUBO
{
	int32_t u_mode;
	float u_near;
	float u_far;
	float _pad0;
};

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

	UBOGL ubo_{ UBOBinding::DebugPass };
	DebugPassUBO debugPassUBO_;

};

#endif
