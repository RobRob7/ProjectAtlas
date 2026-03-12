#ifndef DEBUG_PASS_H
#define DEBUG_PASS_H

#include "constants.h"

#include "ubo_gl.h"

#include <memory>
#include <cstdint>

class Shader;

using namespace Debug_Constants;

class DebugPass
{
public:
	DebugPass();
	~DebugPass();

	void init();
	void destroyGL();
	void render(
		uint32_t normalTex, 
		uint32_t depthTex, 
		float nearPlane, 
		float farPlane, 
		int mode
	);

private:
	uint32_t vao_{};
	std::unique_ptr<Shader> debugShader_;

	UBOGL ubo_{ UBOBinding::DebugPass };
	DebugPassUBO debugPassUBO_{};
};

#endif
