#include "application.h"

//--- PUBLIC ---//
Application::Application(int width, int height, const char* windowTitle)
	: width_(width), height_(height), windowTitle_(windowTitle)
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

	// WINDOW CREATION + CHECK
	window_ = glfwCreateWindow(width_, height_, windowTitle_, nullptr, nullptr);
	if (!window_)
	{
		glfwTerminate();
		throw std::runtime_error("GLFW window creation failure!");
	} // end if

	// tell GLFW to make the context of our window the main context on the current thread
	glfwMakeContextCurrent(window_);

	// vsync
	glfwSwapInterval(1);

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
			auto* self = static_cast<Application*>(glfwGetWindowUserPointer(window));
			if (!self) return;

			self->width_ = width;
			self->height_ = height;

			glViewport(0, 0, width, height);

			if (self->scene_)
			{
				self->scene_->onResize(self->width_, self->height_);
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

	// imgui init
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window_, true);
	ImGui_ImplOpenGL3_Init("#version 460 core");
	
	// setup scene
	scene_ = std::make_unique<Scene>(width_, height_);
	scene_->init();
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
		float currentFrame = glfwGetTime();
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

		// imgui start
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// process user input
		InputState input = buildInputState();
		scene_->update(deltaTime_, input);

		// process window close request
		if (input.quitRequested)
		{
			glfwSetWindowShouldClose(window_, true);
		}

		// process camera active/inactive mouse cursor
		if (input.enableCameraPressed)
		{
			glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		if (input.disableCameraPressed)
		{
			glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		///////////////////////////////////

		// render scene
		scene_->render(glfwGetTime());

		// draw inspector (on top)
		scene_->drawImGui();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// swap buffers
		glfwSwapBuffers(window_);
	} // end while
} // end of run()

//--- PRIVATE ---//
InputState Application::buildInputState()
{
	InputState in{};

	// graphics options
	static bool spaceWasDown = false;
	bool spaceDown = glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_PRESS;
	in.enableSSAO = (spaceDown && !spaceWasDown);
	spaceWasDown = spaceDown;

	// debug
	in.debugOffPressed = (glfwGetKey(window_, GLFW_KEY_1) == GLFW_PRESS);
	in.debugNormalPressed = (glfwGetKey(window_, GLFW_KEY_2) == GLFW_PRESS);
	in.debugDepthPressed = (glfwGetKey(window_, GLFW_KEY_3) == GLFW_PRESS);

	// quit
	in.quitRequested = (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS);

	// camera enable/disable (edge-trigger optional; using "held" is okay but can spam)
	in.disableCameraPressed = (glfwGetKey(window_, GLFW_KEY_MINUS) == GLFW_PRESS);
	in.enableCameraPressed = (glfwGetKey(window_, GLFW_KEY_EQUAL) == GLFW_PRESS);

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