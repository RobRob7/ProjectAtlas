#ifndef SHADOW_MAP_PASS_VK_H
#define SHADOW_MAP_PASS_VK_H

#include "constants.h"
#include "bindings.h"

#include "image_vk.h"
#include "buffer_vk.h"
#include "descriptor_set_vk.h"
#include "graphics_pipeline_vk.h"

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

#include <memory>

using namespace Shadow_Map_Constants;

class VulkanMain;
class ShaderModuleVk;
class ChunkPassVk;
struct RenderInputs;
struct FrameContext;

class ShadowMapPassVk
{
public:
	explicit ShadowMapPassVk(VulkanMain& vk);
	~ShadowMapPassVk();

	void init();

	void renderOffscreen(
		ChunkPassVk& chunk,
		const RenderInputs& in,
		const FrameContext& frame
	);

	ImageVk& getImage() { return depthImage_; }

	const glm::mat4& getLightSpaceMatrix() const { return lightSpaceMatrix_; }

private:
	void buildLightSpaceBounds(
		const RenderInputs& in,
		const glm::vec3& minWS,
		const glm::vec3& maxWS
	);
	void createAttachments();
	void createResources();
	void createDescriptorSet();
	void createPipeline();
private:
	VulkanMain& vk_;
	int width_{ SHADOW_RESOLUTION };
	int height_{ SHADOW_RESOLUTION };

	glm::mat4 lightSpaceMatrix_{};
	glm::mat4 lightView_{};
	glm::mat4 lightProj_{};

	ImageVk depthImage_;

	std::unique_ptr<ShaderModuleVk> shader_;

	ShadowMapPassUBO uboData_{};

	BufferVk uboBuffer_;
	DescriptorSetVk descriptorSet_;
	GraphicsPipelineVk pipeline_;

	vk::ImageLayout depthLayout_ = vk::ImageLayout::eUndefined;
	vk::Format depthFormat_ = vk::Format::eD32Sfloat;
};

#endif
