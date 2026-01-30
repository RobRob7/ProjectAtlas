#ifndef GBUFFERPASS_H
#define GBUFFFERPASS_H

#include "chunkmanager.h"
#include "shader.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <cstdint>
#include <stdexcept>
#include <optional>

class GBufferPass
{
public:
	~GBufferPass();
	void init();
	void resize(float w, float h);
	void destroyGL();

	void render(ChunkManager& world, const glm::mat4& view, const glm::mat4& proj);

	uint32_t getNormalTexture() const;
	uint32_t getDepthTexture() const;
	uint32_t getFBO() const;

private:
	int width_{};
	int height_{};

	std::optional<Shader> gBufferShader_;

	uint32_t fbo_{};
	uint32_t gNormalTexture_{};
	uint32_t gDepthTexture_{};
private:
	void createTargets();
	void destroyTargets();
};

#endif
