#ifndef CHUNK_OPAQUE_PASS_GL_H
#define CHUNK_OPAQUE_PASS_GL_H

#include "shader.h"
#include "texture.h"

#include <glm/glm.hpp>

#include <memory>

class Shader;
class Texture;

struct RenderInputs;
struct RenderSettings;
struct ChunkDrawList;

class ChunkOpaquePassGL
{
public:
	ChunkOpaquePassGL() = default;
	~ChunkOpaquePassGL();

	void init();
	void updateShader(const RenderInputs& in, const RenderSettings& rs, const int w, const int h);

	void renderOpaque(
		const RenderInputs& in,
		const glm::mat4& view,
		const glm::mat4& proj,
		int width, int height);
	void renderOpaque(
		Shader& shader,
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

private:
	std::unique_ptr<Shader> opaqueShader_;
	std::unique_ptr<Shader> waterShader_;
	std::unique_ptr<Texture> atlas_;
};

#endif
