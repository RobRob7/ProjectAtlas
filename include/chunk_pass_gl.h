#ifndef CHUNK_PASS_GL_H
#define CHUNK_PASS_GL_H

#include "constants.h"
#include "ubo_gl.h"

#include "shader.h"
#include "texture.h"

#include <glm/glm.hpp>

#include <memory>
#include <cstdint>

struct RenderInputs;
struct RenderSettings;
struct ChunkDrawList;

using namespace Chunk_Constants;

class ChunkPassGL
{
public:
	ChunkPassGL() = default;
	~ChunkPassGL();

	void init();
	void updateShader(const RenderInputs& in, const RenderSettings& rs, const int w, const int h);

	void renderOpaque(
		const RenderInputs& in,
		const glm::mat4& view,
		const glm::mat4& proj,
		int width, int height);
	void renderOpaque(
		Shader& shader,
		UBOGL& uboGL,
		void* ubo,
		uint32_t uboSize,
		glm::vec3& chunkOrigin,
		const RenderInputs& in,
		const glm::mat4& view,
		const glm::mat4& proj,
		int width, int height);
	void renderWater(
		const RenderInputs& in,
		const glm::mat4& view,
		const glm::mat4& proj,
		int width, int height);

	Shader& getOpaqueShader() { return *opaqueShader_; }
	Shader& getWaterShader() { return *waterShader_; }
	ChunkOpaqueUBO& getOpaqueUBO() { return chunkOpaqueUBO_; }
	ChunkWaterUBO& getWaterUBO() { return chunkWaterUBO_; }

private:
	std::unique_ptr<Shader> opaqueShader_;
	std::unique_ptr<Shader> waterShader_;
	std::unique_ptr<Texture> atlas_;
	UBOGL uboOpaque_{ UBOBinding::Chunk };
	ChunkOpaqueUBO chunkOpaqueUBO_;
	UBOGL uboWater_{ UBOBinding::WaterPass };
	ChunkWaterUBO chunkWaterUBO_;

};

#endif