#ifndef GBUFFERPASS_H
#define GBUFFERPASS_H

#include "ubo_gl.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <memory>

class ChunkPassGL;
class Shader;
class ChunkManager;

struct RenderInputs;

struct GbufferUBO
{
	glm::mat4 u_view;
	glm::mat4 u_proj;

	glm::vec3 u_chunkOrigin;
	float _pad0;
};

class GBufferPass
{
public:
	GBufferPass();
	~GBufferPass();

	void init();
	void resize(int w, int h);

	void render(
		ChunkPassGL& chunk,
		const RenderInputs& in, 
		const glm::mat4& view, 
		const glm::mat4& proj);

	uint32_t getNormalTexture() const;
	uint32_t getDepthTexture() const;
	uint32_t getFBO() const;

private:
	int width_{};
	int height_{};

	std::unique_ptr<Shader> gBufferShader_;

	uint32_t fbo_{};
	uint32_t gNormalTexture_{};
	uint32_t gDepthTexture_{};

	UBOGL ubo_{ UBOBinding::Gbuffer };
	GbufferUBO gbufferUBO_{};
private:
	void createTargets();
	void destroyTargets();
	void destroyGL();
};

#endif
