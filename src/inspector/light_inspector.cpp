#include "light_inspector.h"

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