#ifndef RENDERER_VK_H
#define RENDERER_VK_H

#include "i_renderer.h"

#include "chunk_pass_vk.h"

#include <memory>

class VulkanMain;
class Camera;
class ILight;
class ICubemap;
struct RenderInputs;
struct RenderSettings;

class RendererVk final : public IRenderer
{
public:
	explicit RendererVk(VulkanMain& vk);
	~RendererVk() override;

	void init() override;
	void resize(int w, int h) override;
	void renderFrame(const RenderInputs& in, const FrameContext& frame, UIVk* ui) override;

	RenderSettings& settings();

private:

private:
	int width_{};
	int height_{};

	VulkanMain& vk_;

	std::unique_ptr<RenderSettings> renderSettings_;

	std::unique_ptr<ChunkPassVk> chunkPass_;
};

#endif
