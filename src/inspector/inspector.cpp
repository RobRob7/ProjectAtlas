#include "inspector.h"

void DrawStatsWindow(float dt)
{
	ImGuiViewport* vp = ImGui::GetMainViewport();

	ImVec2 pos = vp->WorkPos;
	pos.x += 10.0f;
	pos.y += 36.0f;

	ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.35f);

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("##FPSOverlay", nullptr, flags))
	{
		float ms = dt * 1000.0f;
		float fps = (dt > 0.0f) ? (1.0f / dt) : 0.0f;

		ImGui::Text("FPS: %.1f", fps);
		ImGui::Text("Frame: %.3f ms", ms);
	}
	ImGui::End();
} // end of DrawStatsWindow()

void DrawLightInspector(Light& light)
{
	glm::vec3 pos = light.getPosition();
	glm::vec3 color = light.getColor();

	bool changed = false;

	changed |= ImGui::DragFloat3("Position", glm::value_ptr(pos), 0.1f);
	if (ImGui::Button("Reset##pos"))
	{
		light.setPosition(glm::vec3(0.0f, 100.0f, 3.0f));
	}
	changed |= ImGui::ColorEdit3("Color", glm::value_ptr(color));
	if (ImGui::Button("Reset##Color"))
	{
		light.setColor(glm::vec3(1.0f));
	}

	if (changed)
	{
		light.setPosition(pos);
		light.setColor(color);
	}

	ImGui::Separator();
} // end of DrawLightInspector()

void DrawWorldInspector(ChunkManager& world)
{
	bool changed = false;

	float ambientStrength = world.getAmbientStrength();
	changed |= ImGui::DragFloat("Ambient Strength", &ambientStrength, 0.01f, 0.0f, 0.5f);
	if (ImGui::Button("Reset##amb"))
	{
		ambientStrength = 0.5f;
		world.setAmbientStrength(ambientStrength);
	}

	if (changed)
	{
		world.setAmbientStrength(ambientStrength);
	}

	ImGui::Separator();
} // end of DrawWorldInspector()

void DrawRendererInspector(RenderInputs& rendererInputs)
{

} // end of DrawRendererInspector()