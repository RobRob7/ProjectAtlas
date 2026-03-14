#ifndef CUBEMAP_VK_H
#define CUBEMAP_VK_H

#include "constants.h"

#include "i_cubemap.h"

#include "shader_vk.h"
#include "buffer_vk.h"
#include "descriptor_set_vk.h"
#include "graphics_pipeline_vk.h"
#include "texture_cubemap_vk.h"

#include <glm/glm.hpp>

#include <memory>
#include <string_view>
#include <array>
#include <cstdint>

class VulkanMain;

class CubemapVk final : public ICubemap
{
public:
    CubemapVk(VulkanMain& vk, const std::array<std::string_view, 6>& textures = Cubemap_Constants::DEFAULT_FACES);
    ~CubemapVk() override;

    void init() override;

    void render(
		const FrameContext* frame,
		const glm::mat4& view,
		const glm::mat4& projection,
		const float time = -1.0
	) override;
	void renderOffscreen(
		const FrameContext* frame,
		const glm::mat4& view,
		const glm::mat4& projection,
		const float time = -1.0
	);

private:
	void createVertexBuffer();
	void createUBO();
	void createDescriptorSet();
	void createPipeline();
private:
	VulkanMain& vk_;

	std::array<std::string_view, 6> faces_;

	std::unique_ptr<ShaderModuleVk> shader_;

	TextureCubemapVk cubemapTexture_;

	BufferVk uboBuffer_;
	BufferVk vertexBuffer_;

	uint32_t vertexCount_{};

	DescriptorSetVk descriptorSet_;

	GraphicsPipelineVk pipeline_;
	GraphicsPipelineVk pipelineOffscreen_;
};

#endif
