#ifndef DEBUG_PASS_VK_H
#define DEBUG_PASS_VK_H

#include "buffer_vk.h"
#include "descriptor_set_vk.h"
#include "graphics_pipeline_vk.h"

#include <memory>

class VulkanMain;
class GBufferPassVk;
class ShaderModuleVk;
struct FrameContext;
class ImageVk;

class DebugPassVk
{
public:
	DebugPassVk(VulkanMain& vk, ImageVk& normalImage, ImageVk& depthImage);
	~DebugPassVk();

	void init();
	void resize(int w, int h);

	void render(
		const FrameContext& frame,
		vk::ImageLayout oldLayout,
		float nearPlane,
		float farPlane,
		int mode
	);

private:
	void refreshInputs();
	void createResources();
	void createDescriptorSet();
	void createPipeline();
private:
	VulkanMain& vk_;
	ImageVk& normalImage_;
	ImageVk& depthImage_;
	
	std::unique_ptr<ShaderModuleVk> shader_;

	BufferVk uboBuffer_;
	DescriptorSetVk descriptorSet_;
	GraphicsPipelineVk pipeline_;

	int width_{};
	int height_{};
};

#endif