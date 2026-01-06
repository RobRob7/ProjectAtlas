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

	// window top nav bar logo
	logoTex_ = (ImTextureID)(intptr_t)Texture("blocks.png").m_ID;

	// ------ imgui init ------ //
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window_, true);
	ImGui_ImplOpenGL3_Init("#version 460 core");
	
	// setup scene
	scene_ = std::make_unique<Scene>(width_, height_);
	scene_->init();
} // end of constructor

Application::~Application()
{
	// imgui shutdown
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

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
		in_.time = glfwGetTime();
		in_.useSSAO = renderSettings_.useSSAO;
		in_.debugMode = renderSettings_.debugMode;
		scene_->render(in_);

		// draw UI
		drawFullUI();

		// swap buffers
		glfwSwapBuffers(window_);
	} // end while
} // end of run()

//--- PRIVATE ---//
InputState Application::buildInputState()
{
	InputState in{};
	
	// ------- graphics options -------
	// SSAO
	static bool spaceWasDown = false;
	bool spaceDown = glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_PRESS;
	if (spaceDown && !spaceWasDown)
	{
		renderSettings_.useSSAO = !renderSettings_.useSSAO;
	}
	spaceWasDown = spaceDown;


	// debug
	if (glfwGetKey(window_, GLFW_KEY_1) == GLFW_PRESS)
	{
		renderSettings_.debugMode = DebugMode::None;
	}
	if (glfwGetKey(window_, GLFW_KEY_2) == GLFW_PRESS)
	{
		renderSettings_.debugMode = DebugMode::Normals;
	}
	if (glfwGetKey(window_, GLFW_KEY_3) == GLFW_PRESS)
	{
		renderSettings_.debugMode = DebugMode::Depth;
	}

	// quit
	in.quitRequested = (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS);

	// camera enable/disable
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

void Application::drawFullUI()
{
	drawTopBar(window_, logoTex_);

	drawStatsFPS();

	drawInspector();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup);
	}
} // end of drawUI()

void Application::drawTopBar(GLFWwindow* window, ImTextureID logoTex)
{
	ImGuiViewport* vp = ImGui::GetMainViewport();

	const float barHeight = 36.0f;
	const float padding = 8.0f;
	const float btnSize = 18.0f;

	ImGui::SetNextWindowPos(vp->Pos);
	ImGui::SetNextWindowSize(ImVec2(vp->Size.x, barHeight));

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoSavedSettings;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, 6));
	ImGui::Begin("##TopBar", nullptr, flags);
	ImGui::PopStyleVar();

	// ----- logo -----
	ImGui::Image(logoTex, ImVec2(20, 20));
	ImGui::SameLine();

	// ----- title -----
	ImGui::TextUnformatted("Project Atlas");
	ImGui::SameLine();

	// right-aligned window buttons
	float right = ImGui::GetWindowContentRegionMax().x;

	ImGui::SetCursorPosX(right - (btnSize * 3 + padding * 2));

	// minimize
	if (ImGui::Button("_", ImVec2(btnSize, btnSize)))
		glfwIconifyWindow(window);

	ImGui::SameLine();

	// maximize/restore
	if (ImGui::Button("[]", ImVec2(btnSize, btnSize)))
	{
		if (glfwGetWindowAttrib(window, GLFW_MAXIMIZED))
			glfwRestoreWindow(window);
		else
			glfwMaximizeWindow(window);
	}

	ImGui::SameLine();

	// close
	if (ImGui::Button("X", ImVec2(btnSize, btnSize)))
		glfwSetWindowShouldClose(window, true);

	// ----- window dragging -----
	if (ImGui::IsWindowHovered() &&
		ImGui::IsMouseDragging(ImGuiMouseButton_Left))
	{
		double dx = ImGui::GetIO().MouseDelta.x;
		double dy = ImGui::GetIO().MouseDelta.y;

		int wx, wy;
		glfwGetWindowPos(window, &wx, &wy);
		glfwSetWindowPos(window, wx + (int)dx, wy + (int)dy);
	}

	ImGui::End();
} // end of drawTopBar()

void Application::drawStatsFPS()
{
	ImGuiViewport* vp = ImGui::GetMainViewport();

	ImVec2 pos = vp->WorkPos;
	pos.x += 10.0f;
	pos.y += 36.0f;

	ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.35f);

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("##FPSOverlay", nullptr, flags))
	{
		float ms = deltaTime_ * 1000.0f;
		float fps = (deltaTime_ > 0.0f) ? (1.0f / deltaTime_) : 0.0f;

		ImGui::Text("FPS: %.1f", fps);
		ImGui::Text("Frame: %.3f ms", ms);
	}
	ImGui::End();
} // end of drawStatsFPS()

void Application::drawInspector()
{
	ImGui::Begin("Inspector");

	// ------- renderer -------
	if (ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen))
	{
		// render mode
		std::string_view mode = "ERROR!";
		switch (in_.debugMode)
		{
		case DebugMode::None:
			mode = "Default";
			break;
		case DebugMode::Normals:
			mode = "Normals";
			break;
		case DebugMode::Depth:
			mode = "Depth";
			break;
		default:
			break;
		}
		ImGui::Text("Render Mode:\n %s", mode.data());

		// SSAO toggle
		ImGui::Checkbox("SSAO", &renderSettings_.useSSAO);

		ImGui::Separator();
	}

	// ------- light -------
	if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
	{
		Light& light = scene_->getLight();
		glm::vec3 pos = light.getPosition();
		glm::vec3 color = light.getColor();

		bool changed = false;

		changed |= ImGui::DragFloat3("Position", glm::value_ptr(pos), 0.1f);
		if (ImGui::Button("Reset##pos"))
		{
			light.setPosition(glm::vec3(0.0f, 100.0f, 3.0f));
		}
		changed |= ImGui::ColorEdit3("Color", glm::value_ptr(color));
		if (ImGui::Button("Reset##Color"))
		{
			light.setColor(glm::vec3(1.0f));
		}

		if (changed)
		{
			light.setPosition(pos);
			light.setColor(color);
		}

		ImGui::Separator();
	}

	// ------- world -------
	if (ImGui::CollapsingHeader("World", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ChunkManager& world = scene_->getWorld();
		bool changed = false;

		float ambientStrength = world.getAmbientStrength();
		changed |= ImGui::DragFloat("Ambient Strength", &ambientStrength, 0.01f, 0.0f, 0.5f);
		if (ImGui::Button("Reset##amb"))
		{
			ambientStrength = 0.5f;
			world.setAmbientStrength(ambientStrength);
		}

		if (changed)
		{
			world.setAmbientStrength(ambientStrength);
		}

		ImGui::Separator();
	}

	ImGui::End();

} // end of drawInspector()