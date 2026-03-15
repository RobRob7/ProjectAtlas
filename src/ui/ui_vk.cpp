#include "ui_vk.h"

#include "vulkan_main.h"
#include "render_settings.h"

#include "i_scene.h"
#include "i_light.h"

#include "texture_2d_vk.h"

#include "chunk_manager.h"
#include "camera.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vulkan/vulkan.hpp>

#define NOMINMAX
#include <windows.h>
#include <psapi.h>

//--- HELPER ---//
static size_t GetProcessMemoryMB()
{
	PROCESS_MEMORY_COUNTERS_EX pmc{};
	GetProcessMemoryInfo(
		GetCurrentProcess(),
		(PROCESS_MEMORY_COUNTERS*)&pmc,
		sizeof(pmc)
	);

	// Working Set = physical RAM currently used
	return pmc.WorkingSetSize / (1024 * 1024);
} // end of GetProcessMemoryMB()

//--- PUBLIC ---//
UIVk::UIVk(VulkanMain& vk, GLFWwindow* window, RenderSettings& rs, Backend activeBackend)
	: vk_(vk), window_(window), renderSettings_(rs),
	enabled_(true), cameraModeOn_(true),
	activeBackend_(activeBackend),
	selectedBackend_(activeBackend)
{
	// window top nav bar logo
	logoTex_ = std::make_unique<Texture2DVk>(vk_);
	logoTex_->loadFromFile("blocks.png", false);

	// ------ imgui init ------ //
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(window_, true);

	ImGui_ImplVulkan_InitInfo initInfo{};
	initInfo.Instance = static_cast<VkInstance>(vk_.getInstance());
	initInfo.PhysicalDevice = static_cast<VkPhysicalDevice>(vk_.getPhysicalDevice());
	initInfo.Device = static_cast<VkDevice>(vk_.getDevice());
	initInfo.QueueFamily = vk_.getGraphicsQueueFamilyIndex();
	initInfo.Queue = static_cast<VkQueue>(vk_.getGraphicsQueue());
	initInfo.PipelineCache = VK_NULL_HANDLE;
	initInfo.DescriptorPool = static_cast<VkDescriptorPool>(vk_.getImGuiDescriptorPool());
	initInfo.DescriptorPoolSize = 0;
	initInfo.MinImageCount = vk_.getMinImageCount();
	initInfo.ImageCount = vk_.getSwapchainImageCount();
	initInfo.Allocator = nullptr;
	initInfo.CheckVkResultFn = nullptr;

	initInfo.UseDynamicRendering = true;
	initInfo.PipelineInfoMain.Subpass = 0;
	initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	VkFormat colorFormat = static_cast<VkFormat>(vk_.getSwapChainImageFormat());

	VkPipelineRenderingCreateInfoKHR pipelineRenderingInfo{};
	pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
	pipelineRenderingInfo.colorAttachmentCount = 1;
	pipelineRenderingInfo.pColorAttachmentFormats = &colorFormat;
	pipelineRenderingInfo.depthAttachmentFormat = static_cast<VkFormat>(vk_.getDepthFormat());
	pipelineRenderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

	initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = pipelineRenderingInfo;

	ImGui_ImplVulkan_Init(&initInfo);

	// logo
	logoId_ = reinterpret_cast<ImTextureID>(
		ImGui_ImplVulkan_AddTexture(
			logoTex_->sampler(),
			logoTex_->view(),
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		)
		);
} // end of constructor

UIVk::~UIVk()
{
	// imgui shutdown
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
} // end of destructor

void UIVk::beginFrame()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
} // end of beginFrame()

void UIVk::buildUI(float dt, IScene& scene)
{
	drawTopBar();

	if (enabled_)
	{
		drawStatsFPS(dt);
		drawInspector(scene);
	}

	ImGui::Render();
} // end of drawFullUI()

void UIVk::render(vk::CommandBuffer cmd)
{
	ImGui_ImplVulkan_RenderDrawData(
		ImGui::GetDrawData(), 
		cmd
	);
} // end of render()

void UIVk::setUIInputEnabled(bool enabled)
{
	ImGuiIO& io = ImGui::GetIO();

	io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;

	io.WantCaptureKeyboard = enabled;
	io.WantCaptureMouse = enabled;

	io.MouseDrawCursor = enabled;
} // end of setUIInputEnabled()

void UIVk::setUIDisplayEnabled(bool enabled)
{
	enabled_ = enabled;
} // end of setUIDisplayEnabled()

void UIVk::setCameraModeUIEnabled(bool enabled)
{
	cameraModeOn_ = enabled;
} // end of setCameraModeUIEnabled()

std::string_view UIVk::backendToString(Backend backend) const
{
	switch (backend)
	{
	case Backend::OpenGL: return "OpenGL";
	case Backend::Vulkan: return "Vulkan";
	case Backend::DX12:   return "DX12";
	default:              return "Unknown";
	}
} // end of backendToString()

void UIVk::setActiveBackend(Backend backend)
{
	activeBackend_ = backend;
	selectedBackend_ = backend;
} // end of setActiveBackend()

bool UIVk::applyBackendRequest(Backend& outBackend)
{
	if (!backendApplyRequested_)
	{
		return false;
	}

	backendApplyRequested_ = false;
	outBackend = selectedBackend_;
	return true;
} // end of applyBackendRequest()

void UIVk::onSwapchainRecreated()
{
	ImGui_ImplVulkan_SetMinImageCount(vk_.getMinImageCount());
} // end of onSwapchainRecreated()


//--- PRIVATE ---//
void UIVk::drawTopBar()
{
	ImGuiViewport* vp = ImGui::GetMainViewport();

	const float barHeight = 30.0f;
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
	float h = barHeight;
	float aspect = static_cast<float>(logoTex_->width()) / static_cast<float>(logoTex_->height());
	ImGui::Image(logoId_, ImVec2(h * aspect, h));
	ImGui::SameLine(0.0f, 1.0f);

	// ----- title -----
	ImGui::TextUnformatted("Project Atlas");
	ImGui::SameLine();

	// right-aligned window buttons
	float right = ImGui::GetWindowContentRegionMax().x;

	ImGui::SetCursorPosX(right - (btnSize * 3 + padding * 2));

	// minimize
	if (ImGui::Button("_", ImVec2(btnSize, btnSize)))
		glfwIconifyWindow(window_);

	ImGui::SameLine();

	// maximize/restore
	if (ImGui::Button("[]", ImVec2(btnSize, btnSize)))
	{
		if (glfwGetWindowAttrib(window_, GLFW_MAXIMIZED))
			glfwRestoreWindow(window_);
		else
			glfwMaximizeWindow(window_);
	}

	ImGui::SameLine();

	// close
	if (ImGui::Button("X", ImVec2(btnSize, btnSize)))
		glfwSetWindowShouldClose(window_, true);

	// ----- window dragging -----
	if (ImGui::IsWindowHovered() &&
		ImGui::IsMouseDragging(ImGuiMouseButton_Left))
	{
		double dx = ImGui::GetIO().MouseDelta.x;
		double dy = ImGui::GetIO().MouseDelta.y;

		int wx, wy;
		glfwGetWindowPos(window_, &wx, &wy);
		glfwSetWindowPos(window_, wx + (int)dx, wy + (int)dy);
	}

	ImGui::End();
} // end of drawTopBar()

void UIVk::drawStatsFPS(float dt)
{
	ImGuiViewport* vp = ImGui::GetMainViewport();

	const float padding = 10.0f;

	const float renderLeft = vp->Pos.x + INSPECTOR_WIDTH1;
	const float renderTop = vp->Pos.y + TOP_BAR_HEIGHT1;
	const float renderRight = vp->Pos.x + vp->Size.x;

	ImVec2 anchor = ImVec2(renderRight - padding, renderTop + padding);

	ImGui::SetNextWindowViewport(vp->ID);
	ImGui::SetNextWindowPos(anchor, ImGuiCond_Always, ImVec2(1.0f, 0.0f));
	ImGui::SetNextWindowBgAlpha(0.35f);

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("##StatsOverlay", nullptr, flags))
	{
		float ms = dt * 1000.0f;
		float fps = (dt > 0.0f) ? (1.0f / dt) : 0.0f;

		ImGui::Text("FPS: %.1f", fps);
		ImGui::Text("Frametime: %.3f ms", ms);

		ImGui::Separator();
		ImGui::Text("RAM (Working Set): %zu MB", GetProcessMemoryMB());

		ImGui::Separator();
		vk::PhysicalDeviceProperties props = vk_.getPhysicalDeviceProperties();
		ImGui::Text("Device: %s", props.deviceName);
	}
	ImGui::End();
} // end of drawStatsFPS()

void UIVk::drawInspector(IScene& scene)
{
	ImGuiViewport* vp = ImGui::GetMainViewport();

	ImVec2 pos = ImVec2(vp->Pos.x, vp->Pos.y + TOP_BAR_HEIGHT1);
	ImVec2 size = ImVec2(INSPECTOR_WIDTH1, vp->Size.y - TOP_BAR_HEIGHT1);

	ImGui::SetNextWindowViewport(vp->ID);
	ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(size, ImGuiCond_Always);

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoSavedSettings;

	ImGui::Begin("Inspector", nullptr, flags);

	// ------- renderer -------
	if (ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen))
	{
#ifdef _DEBUG
		// render mode
		std::string_view mode = "ERROR!";

		switch (renderSettings_.debugMode)
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

		ImGui::Separator();
#endif

		// backend mode
		ImGui::Text("Backend: %s", backendToString(activeBackend_).data());

		int backendIndex = 0;
		switch (selectedBackend_)
		{
		case Backend::OpenGL: backendIndex = 0; break;
		case Backend::Vulkan: backendIndex = 1; break;
		case Backend::DX12:   backendIndex = 2; break;
		}

		const char* backendItems[] = { "OpenGL", "Vulkan" };

		if (ImGui::Combo("Graphics API##render", &backendIndex, backendItems, 2))
		{
			switch (backendIndex)
			{
			case 0: selectedBackend_ = Backend::OpenGL; break;
			case 1: selectedBackend_ = Backend::Vulkan; break;
			}
		}

		if (selectedBackend_ != activeBackend_)
		{
			ImGui::Text("Backend change pending.");

			if (ImGui::Button("Apply Backend"))
			{
				// save world
				scene.getWorld().saveWorld();

				backendApplyRequested_ = true;
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel Backend Change"))
			{
				selectedBackend_ = activeBackend_;
			}
		}

		ImGui::Separator();

		// camera/cursor mode
		ImGui::Text("Mode:\n %s", cameraModeOn_ ? "Camera" : "Cursor");
		ImGui::Separator();

		// render count + status
		ChunkManager& world = scene.getWorld();
		bool frustumCulling = world.statusFrustumCulling();
		if (ImGui::Checkbox("Frustum Culling##render", &frustumCulling))
		{
			world.enableFrustumCulling(frustumCulling);
		}
		bool distanceCulling = world.statusDistanceCulling();
		if (ImGui::Checkbox("Distance Culling##render", &distanceCulling))
		{
			world.enableDistanceCulling(distanceCulling);
		}
		ImGui::Text("Chunks Rendered: %d", world.getFrameChunksRendered());
		ImGui::Text("Blocks Rendered: %d", world.getFrameBlocksRendered());

		ImGui::Separator();

		// DISPLAY OPTIONS
		ImGui::Text("Display Options:");
		// VSync toggle
		if (ImGui::Checkbox("VSync##render", &renderSettings_.enableVsync))
		{
			//glfwSwapInterval(renderSettings_.enableVsync);
		}

		//// GRAPHICS OPTIONS
		//ImGui::Text("Graphics Options:");
		//// SSAO toggle
		//ImGui::Checkbox("SSAO##render", &renderSettings_.useSSAO);

		//// FXAA toggle
		//ImGui::Checkbox("FXAA##render", &renderSettings_.useFXAA);

		//// Fog toggle
		//ImGui::Checkbox("Fog##render", &renderSettings_.useFog);

		//ImGui::Separator();
	}

	// ------- fog -------
	//if (ImGui::CollapsingHeader("Fog", ImGuiTreeNodeFlags_DefaultOpen))
	//{
	//	bool changed = false;

	//	changed |= ImGui::DragFloat3("Color##fog", glm::value_ptr(renderSettings_.fogSettings.color), 0.1f, 0.0f, 1.0f);
	//	if (ImGui::Button("Reset##fog_color"))
	//	{
	//		renderSettings_.fogSettings.color = glm::vec3{ 1.0f, 1.0f, 1.0f };
	//	}
	//	changed |= ImGui::DragFloat("Start Pos##fog", &renderSettings_.fogSettings.start, 0.1f, 0.0f, renderSettings_.fogSettings.end);
	//	if (ImGui::Button("Reset##fog_start"))
	//	{
	//		renderSettings_.fogSettings.start = 50.0f;
	//	}
	//	changed |= ImGui::DragFloat("End Pos##fog", &renderSettings_.fogSettings.end, 0.1f, renderSettings_.fogSettings.start, 2000.0f);
	//	if (ImGui::Button("Reset##fog_end"))
	//	{
	//		renderSettings_.fogSettings.end = 200.0f;
	//	}

	//	// ensure start + kMinGap <= end ALWAYS
	//	if (changed)
	//	{
	//		const float kMinGap = 100.0f;
	//		const float minFogStart = 25.0f;
	//		if (renderSettings_.fogSettings.start < minFogStart)
	//			renderSettings_.fogSettings.start = minFogStart;

	//		if (renderSettings_.fogSettings.start > renderSettings_.fogSettings.end - kMinGap)
	//		{
	//			renderSettings_.fogSettings.start = std::max(minFogStart, renderSettings_.fogSettings.end - kMinGap);
	//			renderSettings_.fogSettings.end = renderSettings_.fogSettings.start + kMinGap;
	//		}
	//	}
	//	ImGui::Separator();
	//}

	// ------- camera -------
	if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
	{
		Camera& camera = scene.getCamera();
		float movementSpeed = camera.getMovementSpeed();
		glm::vec3 pos = camera.getCameraPosition();
		float fp = camera.getFarPlane();

		bool changed = false;

		changed |= ImGui::DragFloat("Movement Speed##cam", &movementSpeed, 0.1f);
		if (ImGui::Button("Reset##cam_mov"))
		{
			camera.setMovementSpeed(camera.MIN_MOVESPEED);
		}
		changed |= ImGui::DragFloat3("Position##cam", glm::value_ptr(pos), 0.1f);
		if (ImGui::Button("Reset##cam_pos"))
		{
			pos = glm::vec3(0.0f, 100.0f, 3.0f);
			camera.setCameraPosition(pos);
		}
		changed |= ImGui::DragFloat("Far Plane##cam", &fp, 5.0f);
		if (ImGui::Button("Reset##cam_fp"))
		{
			camera.setFarPlane(camera.MIN_FARPLANE);
		}

		if (changed)
		{
			camera.setMovementSpeed(movementSpeed);
			camera.setCameraPosition(pos);
			camera.setFarPlane(fp);
		}
		ImGui::Separator();
	}

	// ------- light -------
	if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ILight& light = scene.getLight();
		glm::vec3 pos = light.getPosition();
		glm::vec3 color = light.getColor();

		bool changed = false;

		changed |= ImGui::DragFloat3("Position##light", glm::value_ptr(pos), 0.1f);
		if (ImGui::Button("Reset##pos"))
		{
			light.setPosition(glm::vec3(0.0f, 100.0f, 3.0f));
		}
		changed |= ImGui::ColorEdit3("Color##light", glm::value_ptr(color));
		if (ImGui::Button("Reset##Color"))
		{
			light.setColor(glm::vec3(Light_Constants::MAX_COLOR));
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
		bool changed = false;

		ChunkManager& world = scene.getWorld();
		float ambientStrength = world.getAmbientStrength();
		changed |= ImGui::DragFloat("Ambient Strength##world", &ambientStrength, 0.01f);
		if (ImGui::Button("Reset##amb"))
		{
			ambientStrength = World::MAX_AMBSTR;
			world.setAmbientStrength(ambientStrength);
		}
		int viewRadius = world.getViewRadius();
		changed |= ImGui::DragInt("View Radius##world", &viewRadius, 1);
		if (ImGui::Button("Reset##radius"))
		{
			viewRadius = World::MIN_RADIUS;
			world.setViewRadius(viewRadius);
		}

		if (changed)
		{
			world.setAmbientStrength(ambientStrength);
			world.setViewRadius(viewRadius);
		}

		ImGui::Separator();
	}

	ImGui::End();
} // end of drawInspector()