#ifndef LIGHT_VK_H
#define LIGHT_VK_H

#include "i_light.h"
#include "buffer_vk.h"
#include "descriptor_set_vk.h"
#include "graphics_pipeline_vk.h"

#include <memory>
#include <algorithm>
#include <cstdint>

class VulkanMain;
class ShaderModuleVk;

class LightVk final : public ILight
{
public:
	LightVk(VulkanMain& vk, const glm::vec3& pos, const glm::vec3& color = glm::vec3(1.0f));
	~LightVk() override;
	
	void init() override;
	void render(const RenderContext& ctx, const glm::mat4& view, const glm::mat4& proj) override;

	glm::vec3& getPosition() override { return position_; }
	const glm::vec3& getPosition() const override { return position_; }
	glm::vec3& getColor() override { return color_; }
	const glm::vec3& getColor() const override { return color_; }

	void setPosition(const glm::vec3& pos) override { position_ = pos; } 

	void setColor(const glm::vec3& color) override
	{
		color_ = {
		std::clamp(color.x, Light_Constants::MIN_COLOR, Light_Constants::MAX_COLOR),
		std::clamp(color.y, Light_Constants::MIN_COLOR, Light_Constants::MAX_COLOR),
		std::clamp(color.z, Light_Constants::MIN_COLOR, Light_Constants::MAX_COLOR)
		};
	} // end of setColor()

private:
	void createVertexBuffer();
	void createUBO();
	void createDescriptorSet();
	void createPipeline();
private:
	VulkanMain& vk_;

	std::unique_ptr<ShaderModuleVk> shader_;

	BufferVk uboBuffer_;
	BufferVk vertexBuffer_;

	uint32_t vertexCount_{};

	DescriptorSetVk descriptorSet_;

	GraphicsPipelineVk pipeline_;

	glm::vec3 position_{};
	glm::vec3 color_{};
};

#endif