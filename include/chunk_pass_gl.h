#ifndef CHUNK_PASS_GL_H
#define CHUNK_PASS_GL_H

#include "ubo_gl.h"

#include "shader.h"
#include "texture.h"

#include <glm/glm.hpp>

#include <memory>

class Shader;
class Texture;

struct RenderInputs;
struct RenderSettings;
struct ChunkDrawList;

struct ChunkOpaqueUBO
{
	glm::vec3 u_chunkOrigin;
	float _pad0;
	glm::mat4 u_view;
	glm::mat4 u_proj;
	glm::vec4 u_clipPlane;

	glm::vec3 u_viewPos;
	float _pad1;
	glm::vec3 u_lightPos;
	float _pad2;
	glm::vec3 u_lightColor;
	float u_ambientStrength;

	glm::vec2 u_screenSize;
	int32_t u_useSSAO;
	int32_t _pad3;
};

struct ChunkWaterUBO
{
	// vert
	glm::mat4 u_model;
	glm::mat4 u_view;
	glm::mat4 u_proj;

	glm::vec4 u_tileScale_pad = glm::vec4{ 0.02f, 0.0f, 0.0f, 0.0f };

	// frag
	float u_time;
	float u_distortStrength = 8.0f;
	float u_waveSpeed = 0.04f;
	float _pad_waves;

	float u_near;
	float u_far;
	glm::vec2 u_screenSize;

	glm::vec3 u_viewPos;
	int32_t _pad0;

	glm::vec3 u_lightPos;
	int32_t _pad1;

	glm::vec3 u_lightColor;
	float u_ambientStrength;
};


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