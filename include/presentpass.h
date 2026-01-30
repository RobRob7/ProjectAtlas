#ifndef PRESENTPASS_H
#define PRESENTPASS_H

#include "shader.h"

#include <glad/glad.h>

#include <cstdint>
#include <optional>

class PresentPass
{
public:
	void init();
	void render(uint32_t sceneColorTex, int w, int h);

private:
	uint32_t fsVao_{};
	std::optional<Shader> shader_;
};

#endif
