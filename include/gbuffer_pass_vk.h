#ifndef GBUFFER_PASS_VK_H
#define GBUFFER_PASS_VK_H

#include "image_vk.h"

#include <glm/glm.hpp>

class VulkanMain;
class ShaderModuleVk;
struct RenderInputs;
struct FrameContext;
class ChunkPassVk;

class GBufferPassVk
{
public:
	explicit GBufferPassVk(VulkanMain& vk);
	~GBufferPassVk();

	void init();
	void resize(int w, int h);

	void render(
		ChunkPassVk& chunk,
		const RenderInputs& in,
		const FrameContext& frame,
		const glm::mat4& view,
		const glm::mat4& proj);

	ImageVk& getNormalImage() { return gNormalImage_; }
	ImageVk& getDepthImage() { return gDepthImage_; }

	vk::ImageView getNormalImageView() const { return gNormalImage_.view(); }
	vk::ImageView getDepthImageView() const { return gDepthImage_.view(); }

	vk::Sampler getNormalSampler() const { return gNormalImage_.sampler(); }
	vk::Sampler getDepthSampler() const { return gDepthImage_.sampler(); }

private:
	void createAttachments();
private:
	VulkanMain& vk_;

	int width_{};
	int height_{};

	ImageVk gNormalImage_;
	ImageVk gDepthImage_;

	vk::ImageLayout normalLayout_ = vk::ImageLayout::eUndefined;
	vk::ImageLayout depthLayout_ = vk::ImageLayout::eUndefined;

	vk::Format normalFormat_ = vk::Format::eR16G16B16A16Sfloat;
	vk::Format depthFormat_ = vk::Format::eD32Sfloat;
};

#endif
