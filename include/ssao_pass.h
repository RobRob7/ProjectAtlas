#ifndef SSAO_PASS_H
#define SSAO_PASS_H

#include "constants.h"

#include "bindings.h"

#include "ubo_gl.h"

#include <glm/glm.hpp>

#include <memory>
#include <array>

class Shader;

class SSAOPass
{
public:
	SSAOPass();
	~SSAOPass();

	void init();
	void resize(int w, int h);
	void destroyGL();

	void render(
		uint32_t normalTex, 
		uint32_t depthTex, 
		const glm::mat4& proj
	);

	uint32_t aoRawTexture() const;
	uint32_t aoBlurTexture() const;

private:
	void createTargets();
	void destroyTargets();
	void createNoise();
	void createKernel();
private:
	int width_{};
	int height_{};

	uint32_t fboRaw_{};
	uint32_t fboBlur_{};
	uint32_t aoRaw_{};
	uint32_t aoBlur_{};
	uint32_t noiseTexture_{};
	
	uint32_t fsVao_{};

	std::unique_ptr<Shader> ssaoShader_;
	std::unique_ptr<Shader> blurShader_;

	UBOGL uboBlur_{ TO_API_FORM(SSAOBlurBinding::UBO) };
	SSAO_Constants::SSAOBlurUBO ssaoBlurUBO_{};
	UBOGL uboSSAO_{ TO_API_FORM(SSAORawBinding::UBO) };
	SSAO_Constants::SSAORawUBO ssaoUBO_{};

	std::array<glm::vec4, SSAO_Constants::MAX_KERNEL_SIZE> samples_{};
};

#endif
