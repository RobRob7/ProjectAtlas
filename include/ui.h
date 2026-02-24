#ifndef UI_H
#define UI_H

#include "i_scene.h"

#include <memory>

class Texture;
//class Scene;
struct GLFWwindow;
struct RenderSettings;

constexpr float TOP_BAR_HEIGHT = 36.0f;
constexpr float INSPECTOR_WIDTH = 400.0f;

class UI
{
public:
	UI(GLFWwindow* window, RenderSettings& rs);
	~UI();

	void init();
	void drawFullUI(float dt, IScene& scene);
	void setUIInputEnabled(bool enabled);
	void setUIDisplayEnabled(bool enabled);
	void setCameraModeUIEnabled(bool enabled);

private:
	void drawTopBar();
	void drawStatsFPS(float dt);
	void drawInspector(IScene& scene);
private:
	GLFWwindow* window_;
	RenderSettings& renderSettings_;

	// window top bar logo
	std::unique_ptr<Texture> logoTex_;
	//void* logoTex_;

	bool enabled_;
	bool cameraModeOn_;
};

#endif