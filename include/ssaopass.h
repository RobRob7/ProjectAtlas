#ifndef SSAOPASS_H
#define SSAOPASS_H

class Shader;

#include <glm/glm.hpp>

#include <memory>
#include <array>

class SSAOPass
{
public:
	SSAOPass();
	~SSAOPass();

	void init();
	void resize(int w, int h);
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
	
	uint32_t fsVao_{};

	std::unique_ptr<Shader> ssaoShader_;
	std::unique_ptr<Shader> blurShader_;

	static constexpr int kNoiseSize_ = 4;
	float radius_ = 5.0f;
	float bias_ = 0.05f;
	static constexpr int MAX_KERNEL_SIZE = 64;
	int kernelSize_ = 64;
	std::array<glm::vec3, MAX_KERNEL_SIZE> samples_{};

private:
	void createTargets();
	void destroyTargets();
	void createNoise();
	void createKernel();
};

#endif
