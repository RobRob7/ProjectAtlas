#ifndef SHADOW_MAP_PASS_GL_H
#define SHADOW_MAP_PASS_GL_H

#include "constants.h"
#include "bindings.h"
#include "ubo_gl.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <memory>

class ChunkPassGL;
class Shader;
class ChunkManager;
struct RenderInputs;

using namespace Shadow_Map_Constants;

class ShadowMapPassGL
{
public:
	ShadowMapPassGL();
	~ShadowMapPassGL();

	void init();

	void renderOffscreen(
		ChunkPassGL& chunk,
		const RenderInputs& in
	);

	uint32_t getDepthTexture() const { return depthTex_; }
	uint32_t getFBO() const { return fbo_; }

	const glm::mat4& getLightSpaceMatrix() const { return lightSpaceMatrix_; }

private:
	int width_{ SHADOW_RESOLUTION };
	int height_{ SHADOW_RESOLUTION };

	glm::mat4 lightSpaceMatrix_{};
	glm::mat4 lightView_{};
	glm::mat4 lightProj_{};

	std::unique_ptr<Shader> shader_;

	uint32_t fbo_{};
	uint32_t depthTex_{};

	UBOGL uboGL_{ TO_API_FORM(ShadowMapPassBinding::UBO) };
	ShadowMapPassUBO uboData_{};

private:
	void buildLightSpaceBounds(
		const RenderInputs& in,
		const glm::vec3& minWS,
		const glm::vec3& maxWS
	);
	void createTargets();
	void destroyTargets();
	void destroyGL();
};

#endif
