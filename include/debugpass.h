#ifndef DEBUGPASS_H
#define DEBUGPASS_H

class Shader;

#include <memory>
#include <cstdint>

class DebugPass
{
public:
	DebugPass();
	~DebugPass();

	DebugPass(const DebugPass&) = delete;
	DebugPass& operator=(const DebugPass&) = delete;
	DebugPass(DebugPass&&) = delete;
	DebugPass& operator=(DebugPass&&) = delete;

	void init();
	void destroyGL();
	void render(uint32_t normalTex, uint32_t depthTex, float nearPlane, float farPlane, int mode);

private:
	uint32_t vao_{};
	std::unique_ptr<Shader> debugShader_;
};

#endif
