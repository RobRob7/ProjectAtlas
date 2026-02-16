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
	void resize(int w, int h);
	void render(uint32_t sceneColorTex);

private:
	void destroyGL();
private:
	int width_{};
	int height_{};

	uint32_t fsVao_{};
	std::unique_ptr<Shader> shader_;
};

#endif
