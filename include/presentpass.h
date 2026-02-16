#ifndef PRESENTPASS_H
#define PRESENTPASS_H

class Shader;

#include <cstdint>
#include <memory>

class PresentPass
{
public:
	~PresentPass();

	void init();
	void render(uint32_t sceneColorTex, int w, int h);

private:
	void destroyGL();
private:
	uint32_t fsVao_{};
	std::unique_ptr<Shader> shader_;
};

#endif
