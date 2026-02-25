#ifndef SSAO_PASS_H
#define SSAO_PASS_H

#include "ubo_gl.h"

#include <glm/glm.hpp>

#include <memory>
#include <array>

inline constexpr int MAX_KERNEL_SIZE = 64;

struct SSAOBlurUBO
{
	glm::vec2 u_texelSize;
	glm::vec2 _pad0;
};

struct SSAOUBO
{
	glm::mat4 u_proj;
	glm::mat4 u_invProj;

	glm::vec2 u_noiseScale;
	float u_radius;
	float u_bias;

	int32_t u_kernelSize;
	float _pad0;
	glm::vec2 _pad1;

	glm::vec3 u_samples[MAX_KERNEL_SIZE];
};

class Shader;

class SSAOPass
{
public:
	SSAOPass();
	~SSAOPass();

	void init();
	void resize(int w, int h);
	void destroyGL();
	void render(uint32_t normalTex, uint32_t depthTex, const glm::mat4& proj);

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
	
	uint32_t fsVao_{};

	std::unique_ptr<Shader> ssaoShader_;
	std::unique_ptr<Shader> blurShader_;

	UBOGL uboBlur_{ UBOBinding::SSAOBlur };
	SSAOBlurUBO ssaoBlurUBO_;
	UBOGL uboSSAO_{ UBOBinding::SSAOPass };
	SSAOUBO ssaoUBO_;

	static constexpr int kNoiseSize_ = 4;
	float radius_ = 5.0f;
	float bias_ = 0.05f;
	int kernelSize_ = 64;
	std::array<glm::vec3, MAX_KERNEL_SIZE> samples_{};

private:
	void createTargets();
	void destroyTargets();
	void createNoise();
	void createKernel();
};

#endif
