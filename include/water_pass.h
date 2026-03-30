#ifndef WATER_PASS_H
#define WATER_PASS_H

#include "constants.h"
#include "bindings.h"
#include "ubo_gl.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <memory>

class ChunkPassGL;
class Texture;
struct RenderInputs;
struct RenderSettings;
class Shader;
class ShadowMapPassGL;

class WaterPass
{
public:
	WaterPass();
	~WaterPass();

	void init();
	void resize(int w, int h);

	void updateShader(
		const RenderInputs& in,
		const RenderSettings& rs,
		const int w, const int h
	);

	void destroyGL();

	void renderOffscreen(
		ShadowMapPassGL* shadowMap,
		ChunkPassGL& chunk,
		const RenderInputs& in
	);

	void renderWater(
		ShadowMapPassGL* shadowMap,
		const RenderInputs& in,
		const glm::mat4& view,
		const glm::mat4& proj,
		int width, int height
	);

	uint32_t getReflColorTex() const;
	uint32_t getRefrColorTex() const;
	uint32_t getRefrDepthTex() const;

	uint32_t getDuDVTex() const;
	uint32_t getNormalTex() const;

private:
	void createTargets();
	void destroyTargets();

	void waterPass(
		ShadowMapPassGL* shadowMap,
		ChunkPassGL& chunk, 
		const RenderInputs& in
	);
	void waterReflectionPass(
		ShadowMapPassGL* shadowMap,
		ChunkPassGL& chunk, 
		const RenderInputs& in
	) const;
	void waterRefractionPass(
		ShadowMapPassGL* shadowMap,
		ChunkPassGL& chunk, 
		const RenderInputs& in
	) const;
private:
	int factor_{};
	int width_{ 0 };
	int height_{ 0 };
	int fullW_{ 0 };
	int fullH_{ 0 };

	std::unique_ptr<Shader> shader_;

	UBOGL ubo_{ TO_API_FORM(WaterBinding::UBO) };
	Chunk_Constants::ChunkWaterUBO waterUBO_;

	uint32_t reflFBO_{0};
	uint32_t reflColorTex_{0};
	uint32_t reflDepthRBO_{0};

	uint32_t refrFBO_{0};
	uint32_t refrColorTex_{0};
	uint32_t refrDepthTex_{0};

	std::unique_ptr<Texture> dudvTex_;
	std::unique_ptr<Texture> normalTex_;
};

#endif
