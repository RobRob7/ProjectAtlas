#ifndef RENDERER_VK_H
#define RENDERER_VK_H

#include "i_renderer.h"

#include <memory>

class VulkanMain;
class Camera;
class ILight;
class ICubemap;
struct RenderInputs;
struct RenderSettings;

class ChunkPassVk;
class GBufferPassVk;
class DebugPassVk;
class WaterPassVk;

class RendererVk final : public IRenderer
{
public:
	explicit RendererVk(VulkanMain& vk);
	~RendererVk() override;

	void init() override;
	void resize(int w, int h) override;

	void renderFrame(
		const RenderInputs& in,
		const FrameContext* pFrame,
		UIVk* ui
	) override;

	RenderSettings& settings();

private:
	int width_{};
	int height_{};

	VulkanMain& vk_;

	std::unique_ptr<RenderSettings> renderSettings_;

	std::unique_ptr<ChunkPassVk> chunkPass_;
	std::unique_ptr<GBufferPassVk> gbufferPass_;
	std::unique_ptr<DebugPassVk> debugPass_;
	std::unique_ptr<WaterPassVk> waterPass_;
};

#endif
