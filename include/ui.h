#ifndef UI_H
#define UI_H

#include "constants.h"

#include <memory>
#include <string_view>

class Texture;
class IScene;
struct GLFWwindow;
struct RenderSettings;

constexpr float TOP_BAR_HEIGHT = 36.0f;
constexpr float INSPECTOR_WIDTH = 400.0f;

class UI
{
public:
	UI(GLFWwindow* window, RenderSettings& rs, Backend activeBackend);
	~UI();

	void beginFrame();
	void drawFullUI(float dt, IScene& scene);
	void setUIInputEnabled(bool enabled);
	void setUIDisplayEnabled(bool enabled);
	void setCameraModeUIEnabled(bool enabled);

	std::string_view backendToString(Backend backend) const;
	void setActiveBackend(Backend backend);
	bool applyBackendRequest(Backend& outBackend);

private:
	void drawTopBar();
	void drawStatsFPS(float dt);
	void drawInspector(IScene& scene);
private:
	Backend activeBackend_ = Backend::OpenGL;
	Backend selectedBackend_ = Backend::OpenGL;
	bool backendApplyRequested_{ false };

	GLFWwindow* window_;
	RenderSettings& renderSettings_;

	// window top bar logo
	std::unique_ptr<Texture> logoTex_;

	bool enabled_;
	bool cameraModeOn_;
};

#endif