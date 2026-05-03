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
	void resize();

	void render(
		ChunkPassVk& chunk,
		const RenderInputs& in,
		const FrameContext& frame,
		const glm::mat4& view,
		const glm::mat4& proj
	);

	const ImageVk& getNormalImage() const { return gNormalImage_; }
	const ImageVk& getDepthImage() const { return gDepthImage_; }

private:
	void createAttachments();
private:
	VulkanMain& vk_;

	ImageVk gNormalImage_;
	vk::Format normalFormat_ = vk::Format::eR16G16B16A16Sfloat;

	ImageVk gDepthImage_;
	vk::Format depthFormat_ = vk::Format::eD32Sfloat;
};

#endif
