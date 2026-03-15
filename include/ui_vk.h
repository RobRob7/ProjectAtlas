#ifndef UI_VK_H
#define UI_VK_H

#include "constants.h"

#include "vulkan/vulkan.hpp"
#include <imgui.h>
#include <imgui_impl_vulkan.h>

#include <string_view>
#include <memory>

class IScene;
struct GLFWwindow;
struct RenderSettings;
struct FrameContext;
class VulkanMain;
class Texture2DVk;

constexpr float TOP_BAR_HEIGHT1 = 36.0f;
constexpr float INSPECTOR_WIDTH1 = 400.0f;

class UIVk
{
public:
	UIVk(VulkanMain& vk, GLFWwindow* window, RenderSettings& rs, Backend activeBackend);
	~UIVk();

	void beginFrame();
	void buildUI(float dt, IScene& scene);
	void render(vk::CommandBuffer cmd, FrameContext& frame);
	void setUIInputEnabled(bool enabled);
	void setUIDisplayEnabled(bool enabled);
	void setCameraModeUIEnabled(bool enabled);

	std::string_view backendToString(Backend backend) const;
	void setActiveBackend(Backend backend);
	bool applyBackendRequest(Backend& outBackend);

	void onSwapchainRecreated();

private:
	void drawTopBar();
	void drawStatsFPS(float dt);
	void drawInspector(IScene& scene);
private:
	Backend activeBackend_ = Backend::OpenGL;
	Backend selectedBackend_ = Backend::OpenGL;
	bool backendApplyRequested_{ false };

	VulkanMain& vk_;

	GLFWwindow* window_;
	RenderSettings& renderSettings_;

	std::unique_ptr<Texture2DVk> logoTex_;
	ImTextureID logoId_;

	bool enabled_;
	bool cameraModeOn_;
};

#endif