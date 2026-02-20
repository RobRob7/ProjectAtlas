#include "ssao_pass.h"

#include "shader.h"

#include <glad/glad.h>

#include <cstdlib>
#include <vector>
#include <stdexcept>
#include <random>
#include <algorithm>

//--- PUBLIC ---//
SSAOPass::SSAOPass() = default;

SSAOPass::~SSAOPass()
{
	destroyGL();
} // end of destructor

void SSAOPass::init()
{
	ssaoShader_ = std::make_unique<Shader>("ssaopass/ssao.vert", "ssaopass/ssao.frag");
	blurShader_ = std::make_unique<Shader>("ssaopass/ssaoblur.vert", "ssaopass/ssaoblur.frag");

	glCreateVertexArrays(1, &fsVao_);

	createKernel();
	createNoise();
} // end of init()

void SSAOPass::resize(int w, int h)
{
	if (w <= 0 || h <= 0) return;
	if (w == width_ && h == height_) return;

	destroyTargets();
	width_ = w;
	height_ = h;
	createTargets();
} // end of resize()

void SSAOPass::destroyGL()
{
	destroyTargets();

	if (noiseTexture_)
	{
		glDeleteTextures(1, &noiseTexture_);
		noiseTexture_ = 0;
	}
	if (fsVao_)
	{
		glDeleteVertexArrays(1, &fsVao_);
		fsVao_ = 0;
	}

	width_ = 0;
	height_ = 0;
} // end of destroyGL()

void SSAOPass::render(uint32_t normalTex, uint32_t depthTex, const glm::mat4& proj, const glm::mat4& invProj)
{
	if (!ssaoShader_ || !blurShader_) return;
	if (!fboRaw_ || !fboBlur_) return;

	glBindFramebuffer(GL_FRAMEBUFFER, fboRaw_);
	glViewport(0, 0, width_, height_);
	glDisable(GL_DEPTH_TEST);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	ssaoShader_->use();
	ssaoShader_->setMat4("u_proj", proj);
	ssaoShader_->setMat4("u_invProj", invProj);
	ssaoShader_->setFloat("u_radius", radius_);
	ssaoShader_->setFloat("u_bias", bias_);
	ssaoShader_->setInt("u_kernelSize", kernelSize_);
	ssaoShader_->setVec2("u_noiseScale", glm::vec2(
		static_cast<float>(width_) / static_cast<float>(kNoiseSize_),
		static_cast<float>(height_) / static_cast<float>(kNoiseSize_)));

	glBindTextureUnit(0, normalTex);
	glBindTextureUnit(1, depthTex);
	glBindTextureUnit(2, noiseTexture_);
	ssaoShader_->setInt("u_gNormal", 0);
	ssaoShader_->setInt("u_gDepth", 1);
	ssaoShader_->setInt("u_noise", 2);

	glBindVertexArray(fsVao_);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// blur
	glBindFramebuffer(GL_FRAMEBUFFER, fboBlur_);
	glViewport(0, 0, width_, height_);
	glClear(GL_COLOR_BUFFER_BIT);

	blurShader_->use();
	blurShader_->setVec2("u_texelSize", glm::vec2(1.0f / width_, 1.0f / height_));
	glBindTextureUnit(0, aoRaw_);
	blurShader_->setInt("u_ao", 0);

	glBindVertexArray(fsVao_);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// restore GL state
	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
} // end of render()

uint32_t SSAOPass::aoRawTexture() const
{
	return aoRaw_;
} // end of aoRawTexture()

uint32_t SSAOPass::aoBlurTexture() const
{
	return aoBlur_;
} // end of aoBlurTexture()

void SSAOPass::setRadius(float r)
{
	radius_ = r;
} // end of setRadius()

void SSAOPass::setBias(float b)
{
	bias_ = b;
} // end of setBias()

void SSAOPass::setKernelSize(int k)
{
	int newKernelSize = std::clamp(k, 1, MAX_KERNEL_SIZE);

	// only update kernel size if different than current
	if (newKernelSize == kernelSize_) return;

	kernelSize_ = newKernelSize;
	
	createKernel();
} // end of setKernelSize()


//--- PRIVATE ---//
void SSAOPass::createTargets()
{
	glCreateFramebuffers(1, &fboRaw_);
	glCreateFramebuffers(1, &fboBlur_);

	glCreateTextures(GL_TEXTURE_2D, 1, &aoRaw_);
	glTextureStorage2D(aoRaw_, 1, GL_R8, width_, height_);
	glTextureParameteri(aoRaw_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(aoRaw_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(aoRaw_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(aoRaw_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glCreateTextures(GL_TEXTURE_2D, 1, &aoBlur_);
	glTextureStorage2D(aoBlur_, 1, GL_R8, width_, height_);
	glTextureParameteri(aoBlur_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(aoBlur_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(aoBlur_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(aoBlur_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glNamedFramebufferTexture(fboRaw_, GL_COLOR_ATTACHMENT0, aoRaw_, 0);
	glNamedFramebufferTexture(fboBlur_, GL_COLOR_ATTACHMENT0, aoBlur_, 0);

	const GLenum bufs[] = { GL_COLOR_ATTACHMENT0 };
	glNamedFramebufferDrawBuffers(fboRaw_, 1, bufs);
	glNamedFramebufferDrawBuffers(fboBlur_, 1, bufs);

	if (glCheckNamedFramebufferStatus(fboRaw_, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("SSAO raw FBO incomplete!");
	}
	if (glCheckNamedFramebufferStatus(fboBlur_, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("SSAO blur FBO incomplete!");
	}
} // end of createTargets()

void SSAOPass::destroyTargets()
{
	if (fboRaw_)
	{
		glDeleteFramebuffers(1, &fboRaw_);
		fboRaw_ = 0;
	}
	if (fboBlur_)
	{
		glDeleteFramebuffers(1, &fboBlur_);
		fboBlur_ = 0;
	}
	if (aoRaw_)
	{
		glDeleteTextures(1, &aoRaw_);
		aoRaw_ = 0;
	}
	if (aoBlur_)
	{
		glDeleteTextures(1, &aoBlur_);
		aoBlur_ = 0;
	}
} // end of destroyTargets()

void SSAOPass::createNoise()
{
	std::mt19937 rng{ std::random_device{}() };
	std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

	std::vector<glm::vec3> noise;
	noise.reserve(kNoiseSize_ * kNoiseSize_);

	for (int i = 0; i < kNoiseSize_ * kNoiseSize_; ++i)
	{
		// rotate around z (tangent plane)
		noise.emplace_back(dist(rng), dist(rng), 0.0f);
	} // end for

	glCreateTextures(GL_TEXTURE_2D, 1, &noiseTexture_);
	glTextureStorage2D(noiseTexture_, 1, GL_RGB16F, kNoiseSize_, kNoiseSize_);
	glTextureSubImage2D(noiseTexture_, 0, 0, 0, kNoiseSize_, kNoiseSize_, GL_RGB, GL_FLOAT, noise.data());

	glTextureParameteri(noiseTexture_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(noiseTexture_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(noiseTexture_, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(noiseTexture_, GL_TEXTURE_WRAP_T, GL_REPEAT);
} // end of createNoise()

void SSAOPass::createKernel()
{
	std::mt19937 rng{ std::random_device{}() };
	std::uniform_real_distribution<float> dist01{ 0.0f,1.0f };

	for (int i = 0; i < kernelSize_; ++i)
	{
		// hemisphere around +z (tangent space)
		glm::vec3 s{
			dist01(rng) * 2.0f - 1.0f,
			dist01(rng) * 2.0f - 1.0f,
			dist01(rng)
		};
		s = glm::normalize(s);
		s *= dist01(rng);

		// bias samples toward the origin
		float scale = static_cast<float>(i) / static_cast<float>(kernelSize_);
		scale = 0.1f + (scale * scale) * (1.0f - 0.1f);
		s *= scale;

		samples_[i] = s;
	} // end for

	// upload kernel
	ssaoShader_->use();
	for (int i = 0; i < kernelSize_; ++i)
	{
		ssaoShader_->setVec3(("u_samples[" + std::to_string(i) + "]").c_str(), samples_[i]);
	} // end for
} // end of createKernel()