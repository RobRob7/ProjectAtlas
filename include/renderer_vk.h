//#ifndef RENDERER_VK_H
//#define RENDERER_VK_H
//
//#include "i_renderer.h"
//
//#include <glm/glm.hpp>
//
//#include <memory>
//
//struct RenderInputs;
//
//enum class DebugMode : int
//{
//	None = 0, // '1' key
//	Normals = 1, // '2' key
//	Depth = 2, // '3' key
//};
//
//struct FogSettings
//{
//	glm::vec3 color{ 1.0f, 1.0f, 1.0f };
//
//	float start = 50.0f;
//	float end = 500.0f;
//};
//
//struct RenderSettings
//{
//	// debug view mode
//	DebugMode debugMode = DebugMode::None;
//
//	// display options
//	bool enableVsync = true;
//
//	// graphics options
//	bool useSSAO = false;
//	bool useFXAA = false;
//	bool useFog = false;
//
//	// fog controls
//	FogSettings fogSettings;
//};
//
//class RendererVk : public IRenderer
//{
//public:
//	VkRenderer();
//	~VkRenderer();
//
//	void init();
//	void resize(int w, int h);
//	void renderFrame(const RenderInputs& in);
//
//	RenderSettings& settings();
//
//private:
//	void destroyGL();
//	void resizeForwardTargets();
//private:
//	int width_{};
//	int height_{};
//
//	std::unique_ptr<RenderSettings> renderSettings_;
//
//	// passes
//	std::unique_ptr<GBufferPass> gbuffer_;
//	std::unique_ptr<DebugPass> debugPass_;
//	std::unique_ptr<SSAOPass> ssaoPass_;
//	std::unique_ptr<FXAAPass> fxaaPass_;
//	std::unique_ptr<FogPass> fogPass_;
//	std::unique_ptr<PresentPass> presentPass_;
//	std::unique_ptr<WaterPass> waterPass_;
//
//	uint32_t forwardFBO_{};
//	uint32_t forwardColorTex_{};
//	uint32_t forwardDepthTex_{};
//};
//
//#endif
