#ifndef RENDERER_VK_H
#define RENDERER_VK_H

#include "i_renderer.h"
#include "render_settings.h"

#include <memory>

class ChunkManager;
class Camera;
class VulkanMain;
//class Light;
//class CubeMap;
//class Crosshair;

class RendererVk final : public IRenderer
{
public:
	explicit RendererVk(VulkanMain& vk);
	~RendererVk() override;

	void init() override;
	void resize(int w, int h) override;
	void renderFrame(const RenderInputs& in) override;

	RenderSettings& settings();

private:

private:
	int width_{};
	int height_{};

	VulkanMain& vk_;

	std::unique_ptr<RenderSettings> renderSettings_;
};

#endif
