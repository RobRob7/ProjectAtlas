#ifndef SSAOPASS_H
#define SSAOPASS_H

#include "shader.h"

#include <glm/glm.hpp>
#include <glad/glad.h>

#include <cstdlib>
#include <optional>
#include <array>
#include <vector>
#include <stdexcept>
#include <random>

class SSAOPass
{
public:
	~SSAOPass();

	void init();
	void resize(float w, float h);
	void destroyGL();
	void render(uint32_t normalTex, uint32_t depthTex, const glm::mat4& proj, const glm::mat4& invProj);

	uint32_t aoRawTexture() const;
	uint32_t aoBlurTexture() const;

	void setRadius(float r);
	void setBias(float b);
	void setKernelSize(int k);

private:
	int width_{};
	int height_{};

	uint32_t fboRaw_{};
	uint32_t fboBlur_{};
	uint32_t aoRaw_{};
	uint32_t aoBlur_{};

	uint32_t noiseTexture_{};
	static constexpr int kNoiseSize_ = 4;

	uint32_t fsVao_{};
	std::optional<Shader> ssaoShader_;
	std::optional<Shader> blurShader_;

	std::array<glm::vec3, 64> samples_{};

	float radius_ = 5.0f;
	float bias_ = 0.05f;
	int kernelSize_ = 64;
private:
	void createTargets();
	void destroyTargets();
	void createNoise();
	void createKernel();
};

#endif
