#ifndef WATERPASS_H
#define WATERPASS_H

#include <cstdint>
#include <memory>

class Texture;
struct RenderInputs;

class WaterPass
{
public:
	WaterPass();
	~WaterPass();

	void init();
	void resize(int w, int h);
	void destroyGL();
	void render(const RenderInputs& in);

	uint32_t getReflColorTex() const;
	uint32_t getRefrColorTex() const;
	uint32_t getRefrDepthTex() const;

	uint32_t getDuDVTex() const;
	uint32_t getNormalTex() const;

private:
	int factor_{ 1 };
	int width_{0};
	int height_{0};
	int fullW_{ 0 };
	int fullH_{ 0 };

	uint32_t reflFBO_{0};
	uint32_t reflColorTex_{0};
	uint32_t reflDepthRBO_{0};

	uint32_t refrFBO_{0};
	uint32_t refrColorTex_{0};
	uint32_t refrDepthTex_{0};

	std::unique_ptr<Texture> dudvTex_;
	std::unique_ptr<Texture> normalTex_;
private:
	void createTargets();
	void destroyTargets();
	void waterPass(const RenderInputs& in);
	void waterReflectionPass(const RenderInputs& in) const;
	void waterRefractionPass(const RenderInputs& in) const;
};

#endif
