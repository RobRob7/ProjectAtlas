#ifndef WATERPASS_H
#define WATERPASS_H

class Texture;

#include "renderinputs.h"

#include <cstdint>
#include <memory>

class WaterPass
{
public:
	WaterPass();
	~WaterPass();

	void init();
	void resize(int w, int h);
	void destroyGL();
	void render(const RenderInputs& in);

	const uint32_t& getReflColorTex() const;
	const uint32_t& getRefrColorTex() const;
	const uint32_t& getRefrDepthTex() const;

	const uint32_t& getDuDVTex() const;
	const uint32_t& getNormalTex() const;

private:
	int factor_{ 2 };
	int width_{};
	int height_{};
	int fullW_{ 0 };
	int fullH_{ 0 };

	uint32_t reflFBO_{};
	uint32_t reflColorTex_{};
	uint32_t reflDepthRBO_{};

	uint32_t refrFBO_{};
	uint32_t refrColorTex_{};
	uint32_t refrDepthTex_{};

	std::unique_ptr<Texture> dudvTex_;
	std::unique_ptr<Texture> normalTex_;
private:
	void createTargets();
	void destroyTargets();
	void waterPass(const RenderInputs& in);
	void waterReflectionPass(const RenderInputs& in);
	void waterRefractionPass(const RenderInputs& in);
};

#endif
