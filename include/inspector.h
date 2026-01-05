#ifndef INSPECTOR_H
#define INSPECTOR_H

#include "light.h"
#include "chunkmanager.h"
#include "renderer.h"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

void DrawLightInspector(Light& light);

void DrawWorldInspector(ChunkManager& world);

void DrawRendererInspector(RenderInputs& rendererInputs);

#endif
