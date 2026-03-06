#ifndef CHUNK_PASS_VK_H
#define CHUNK_PASS_VK_H

#include <glm/glm.hpp>

class VulkanMain;
struct RenderInputs;
struct RenderSettings;
struct DrawContext;
struct FrameContext;

class ChunkPassVk
{
public:
	explicit ChunkPassVk(VulkanMain& vk);
	~ChunkPassVk() = default;

	void renderOpaque(const RenderInputs& in, FrameContext& frame, const glm::mat4& view, const glm::mat4& proj);
	void renderWater(const RenderInputs& in, FrameContext& frame, const glm::mat4& view, const glm::mat4& proj);
private:
	VulkanMain& vk_;
};

#endif
