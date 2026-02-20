#include "Application.h"

#include "scene.h"
#include "renderer.h"
#include "ui.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <memory>
#include <stdexcept>

//--- PUBLIC ---//
Application::Application(int width, int height)
	: width_(width), height_(height)
{
	// intialize GLFW
	if (!glfwInit())
	{
		throw std::runtime_error("GLFW initialization error!");
	}

	// specify major OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	// specify minor OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	// specify OpenGL core-profile
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// disable top bar
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	// allow resizing
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	// WINDOW CREATION + CHECK
	window_ = glfwCreateWindow(width_, height_, "", nullptr, nullptr);
	if (!window_)
	{
		glfwTerminate();
		throw std::runtime_error("GLFW window creation failure!");
	} // end if

	// tell GLFW to make the context of our window the main context on the current thread
	glfwMakeContextCurrent(window_);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// INITIALIZE GLAD + CHECK
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		throw std::runtime_error("GLAD initialization failure!");
	} // end if

	// set callbacks
	glfwSetWindowUserPointer(window_, this);
	glfwSetFramebufferSizeCallback(window_, [](GLFWwindow* window, int width, int height)
		{
			// resize check
#ifdef _DEBUG
			printf("[RESIZE] fb = %d x %d\n", width, height);
#endif

			auto* self = static_cast<Application*>(glfwGetWindowUserPointer(window));
			if (!self) return;

			self->width_ = width;
			self->height_ = height;

			glViewport(0, 0, self->width_, self->height_);

			if (self->scene_)
			{
				self->scene_->onResize(self->width_, self->height_);
			}
			if (self->renderer_)
			{
				self->renderer_->resize(self->width_, self->height_);
			}
		});
	glfwSetCursorPosCallback(window_, [](GLFWwindow* window, double xposIn, double yposIn)
		{
			auto* self = static_cast<Application*>(glfwGetWindowUserPointer(window));
			if (!self || !self->scene_) return;

			self->scene_->onMouseMove(static_cast<float>(xposIn),
				static_cast<float>(yposIn));
		});
	glfwSetScrollCallback(window_, [](GLFWwindow* window, double xoffset, double yoffset)
		{
			auto* self = static_cast<Application*>(glfwGetWindowUserPointer(window));
			if (!self || !self->scene_) return;


			self->scene_->onScroll(static_cast<float>(yoffset));
		});

	// enable depth test
	glEnable(GL_DEPTH_TEST);

	// setup scene + renderer
	scene_ = std::make_unique<Scene>(width_, height_);
	renderer_ = std::make_unique<Renderer>();
	scene_->init(*renderer_);

	// setup UI
	ui_ = std::make_unique<UI>(window_, renderer_->settings());
} // end of constructor

Application::~Application()
{
	// destroy window if still active
	if (window_) glfwDestroyWindow(window_);
	glfwTerminate();
} // end of destructor

void Application::run()
{
	while (!glfwWindowShouldClose(window_))
	{
		///////// BEFORE RENDER ///////////
		// per-frame time logic
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime_ = currentFrame - lastFrame_;
		lastFrame_ = currentFrame;

		// update save timer
		saveTimer_ += deltaTime_;

		// auto saving
		if (saveTimer_ >= (autoSaveTime_ * 60.0f))
		{
			scene_->requestSave();
			saveTimer_ = 0.0f;
		}

		// poll user input events
		glfwPollEvents();

		// UI init
		ui_->init();

		// process user input
		InputState input = buildInputState();
		scene_->update(deltaTime_, input);

		// process window close request
		if (input.quitRequested)
		{
			glfwSetWindowShouldClose(window_, true);
		}

		// process UI display
		if (input.enableImguiPressed)
		{
			ui_->setUIDisplayEnabled(true);
		}
		if (input.disableImguiPressed)
		{
			ui_->setUIDisplayEnabled(false);
		}

		// process camera active/inactive mouse cursor
		if (input.enableCameraPressed)
		{
			glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			ui_->setCameraModeUIEnabled(true);
			ui_->setUIInputEnabled(false);
		}
		if (input.disableCameraPressed)
		{
			glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			ui_->setCameraModeUIEnabled(false);
			ui_->setUIInputEnabled(true);
		}
		///////////////////////////////////

		// render scene
		in_.time = static_cast<float>(glfwGetTime());
		scene_->render(*renderer_, in_);

		// draw UI
		ui_->drawFullUI(deltaTime_, *scene_);

		// swap buffers
		glfwSwapBuffers(window_);
	} // end while
} // end of run()

//--- PRIVATE ---//
InputState Application::buildInputState()
{
	// TEMP: allows keyboard inputs to change state
	// of view mode and SSAO
	// Debug view is not a feature to be used for release
	// SSAO toggle should be mainly controlled through UI
	RenderSettings& settings = renderer_->settings();

#ifdef _DEBUG
	// ------- graphics options -------
	// SSAO
	static bool spaceWasDown = false;
	bool spaceDown = glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_PRESS;
	if (spaceDown && !spaceWasDown)
	{
		settings.useSSAO = !settings.useSSAO;
	}
	spaceWasDown = spaceDown;

	// debug
	if (glfwGetKey(window_, GLFW_KEY_1) == GLFW_PRESS)
	{
		settings.debugMode = DebugMode::None;
	}
	if (glfwGetKey(window_, GLFW_KEY_2) == GLFW_PRESS)
	{
		settings.debugMode = DebugMode::Normals;
	}
	if (glfwGetKey(window_, GLFW_KEY_3) == GLFW_PRESS)
	{
		settings.debugMode = DebugMode::Depth;
	}
#endif

	InputState in{};

	// quit
	in.quitRequested = (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS);

	// camera enable/disable
	in.disableCameraPressed = (glfwGetKey(window_, GLFW_KEY_MINUS) == GLFW_PRESS);
	in.enableCameraPressed = (glfwGetKey(window_, GLFW_KEY_EQUAL) == GLFW_PRESS);

	// UI display
	in.disableImguiPressed = (glfwGetKey(window_, GLFW_KEY_LEFT) == GLFW_PRESS);
	in.enableImguiPressed = (glfwGetKey(window_, GLFW_KEY_RIGHT) == GLFW_PRESS);

	// movement
	in.w = (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS);
	in.s = (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS);
	in.a = (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS);
	in.d = (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS);
	in.sprint = (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);

	// LMB
	int leftState = glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT);
	in.removeBlockPressed = (leftState == GLFW_PRESS && !leftMouseDown_);
	leftMouseDown_ = (leftState == GLFW_PRESS);
	// RMB
	int rightState = glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT);
	in.placeBlockPressed = (rightState == GLFW_PRESS && !rightMouseDown_);
	rightMouseDown_ = (rightState == GLFW_PRESS);

	return in;
} // end of buildInputState()