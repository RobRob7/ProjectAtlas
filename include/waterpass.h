#ifndef WATERPASS_H
#define WATERPASS_H

#include "renderinputs.h"
#include "chunkmanager.h"
#include "camera.h"
#include "light.h"
#include "cubemap.h"

#include "texture.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <cstdint>
#include <stdexcept>
#include <optional>

class WaterPass
{
public:
	~WaterPass();

	void init();
	void resize(int w, int h);
	void destroyGL();
	void render(const RenderInputs& in);

	const uint32_t& getReflColorTex() const;
	const uint32_t& getRefrColorTex() const;
	const uint32_t& getRefrDepthTex() const;

	const uint32_t& getDuDVTex() const;

private:
	int width_{};
	int height_{};

	uint32_t reflFBO_{};
	uint32_t reflColorTex_{};
	uint32_t reflDepthRBO_{};

	uint32_t refrFBO_{};
	uint32_t refrColorTex_{};
	uint32_t refrDepthTex_{};

	std::optional<Texture> dudvTex_;
private:
	void createTargets();
	void destroyTargets();
	void waterPass(const RenderInputs& in);
	void waterReflectionPass(const RenderInputs& in);
	void waterRefractionPass(const RenderInputs& in);
};

#endif
