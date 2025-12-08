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
			if (self)
			{
				self->width_ = width;
				self->height_ = height;
				glViewport(0, 0, width, height);
			}
		});
	glfwSetCursorPosCallback(window_, [](GLFWwindow* window, double xposIn, double yposIn)
		{
			auto* self = static_cast<Application*>(glfwGetWindowUserPointer(window));
			if (self && self->camera_)
			{
				self->camera_->handleMousePosition(
					static_cast<float>(xposIn),
					static_cast<float>(yposIn)
				);
			}
		});
	glfwSetScrollCallback(window_, [](GLFWwindow* window, double xoffset, double yoffset)
		{
			auto* self = static_cast<Application*>(glfwGetWindowUserPointer(window));
			if (self && self->camera_)
			{
				self->camera_->handleMouseScroll(static_cast<float>(yoffset));
			}
		});

	////////// IMGUI //////////
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	// enable keyboard controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	// enable docking
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// setup glfw/opengl backends
	ImGui_ImplGlfw_InitForOpenGL(window_, true);
	ImGui_ImplOpenGL3_Init("#version 460");
	///////////////////////////

	// set viewport
	glViewport(0, 0, width_, height_);

	// enable depth test
	glEnable(GL_DEPTH_TEST);

	// camera controller
	camera_ = std::make_unique<Camera>(width_, height_, glm::vec3(0.0f, 10.0f, 3.0f));

	// cubemap init
	std::vector<std::string> faces
	{
		"texture/cubemap/space_alt/right.png",
		"texture/cubemap/space_alt/left.png",
		"texture/cubemap/space_alt/top.png",
		"texture/cubemap/space_alt/bottom.png",
		"texture/cubemap/space_alt/front.png",
		"texture/cubemap/space_alt/back.png"
	};
	skybox_ = std::make_unique<CubeMap>(faces);

	// crosshair init
	const float crosshairSize = 0.004f;
	crosshair_ = std::make_unique<Crosshair>(crosshairSize);

	//
	world_.initShaderTexture();
} // end of constructor

Application::~Application()
{
	// destroy window if still active
	if (window_) glfwDestroyWindow(window_);
	glfwTerminate();

	// IMGUI shutdown
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
} // end of destructor

void Application::run()
{
	while (!glfwWindowShouldClose(window_))
	{
		// per-frame time logic
		float currentFrame = glfwGetTime();
		deltaTime_ = currentFrame - lastFrame_;
		lastFrame_ = currentFrame;

		// poll user input events
		glfwPollEvents();

		// process user input
		processInput();

		// IMGUI
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		ImGui::Begin("Hello Window");
		ImGui::Text("Select Block");
		if (ImGui::Button("Dirt"))
		{
			world_.setLastBlockUsed(BlockID::Dirt);
		}
		if (ImGui::Button("Stone"))
		{
			world_.setLastBlockUsed(BlockID::Stone);
		}
		if (ImGui::Button("Glow"))
		{
			world_.setLastBlockUsed(BlockID::Glow_Block);
		}
		if (ImGui::Button("Tree Leaf"))
		{
			world_.setLastBlockUsed(BlockID::Tree_Leaf);
		}
		if (ImGui::Button("SAVE"))
		{
			world_.saveWorld();
		}
		ImGui::End();

		// update world
		world_.update(camera_->getCameraPosition());

		// set color to display after clear (state-setting function)
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		// clear the screen colors (state-using function)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//////////////////////////////
		// init transform matrices
		glm::mat4 view = camera_->getViewMatrix();
		// calculate the max distance of live chunks to see
		const float farPlane = world_.getViewRadius() * CHUNK_SIZE * sqrt(2);
		glm::mat4 projection = camera_->getProjectionMatrix(width_ / height_, 0.1f, farPlane);

		world_.getShader().use();
		world_.getShader().setVec3("u_viewPos", camera_->getCameraPosition());
		static float t = 0.0f;
		t += deltaTime_;
		//lightPos_.z += sin(t * 1.0f) * 1.1f;
		world_.getShader().setVec3("u_lightPos", lightPos_);
		world_.getShader().setVec3("u_lightColor", lightColor_);
		
		// render world
		world_.render(view, projection);

		// render skybox
		skybox_->render(view, projection, glfwGetTime());

		// render crosshair
		crosshair_->render();
		//////////////////////////////

		// IMGUI
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// swap buffers
		glfwSwapBuffers(window_);
	} // end while
} // end of run()

//--- PRIVATE ---//
void Application::processInput()
{
	// press 'esc' key to close window
	if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		world_.saveWorld();
		glfwSetWindowShouldClose(window_, true);
	}

	// press 'down' arrow key to disable camera
	if (glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		camera_->setEnabled(false);
		glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	// press 'up' arrow key to enable camera
	if (glfwGetKey(window_, GLFW_KEY_UP) == GLFW_PRESS)
	{
		camera_->setEnabled(true);
		glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	const float speedIncrease = 20.0f;
	// camera speed increase
	if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		camera_->processKeyboard(FORWARD, deltaTime_ * speedIncrease);
	}

	//////////////////////////////////////////////////////////////
	// destroy block check
	int leftState = glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT);
	bool leftJustPressed = (leftState == GLFW_PRESS && !leftMouseDown_);
	if (leftJustPressed && camera_->isEnabled())
	{
		world_.placeOrRemoveBlock(false, camera_->getCameraPosition(), camera_->getCameraDirection());
	}
	leftMouseDown_ = (leftState == GLFW_PRESS);

	// place block check
	int rightState = glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT);
	bool rightJustPressed = (rightState == GLFW_PRESS && !rightMouseDown_);
	if (rightJustPressed && camera_->isEnabled())
	{
		world_.placeOrRemoveBlock(true, camera_->getCameraPosition(), camera_->getCameraDirection());
	}
	rightMouseDown_ = (rightState == GLFW_PRESS);
	//////////////////////////////////////////////////////////////

	// camera change (W A S D)
	if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera_->processKeyboard(FORWARD, deltaTime_);
	}
	if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera_->processKeyboard(BACKWARD, deltaTime_);
	}
	if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera_->processKeyboard(LEFT, deltaTime_);
	}
	if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera_->processKeyboard(RIGHT, deltaTime_);
	}
} // end of processInput()