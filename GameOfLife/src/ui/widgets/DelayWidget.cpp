#include <algorithm>
#include <imgui/imgui.h>

#include "DelayWidget.h"
#include "GameEnums.h"
#include "SimulationControlResult.h"

gol::SimulationControlResult gol::DelayWidget::Update(EditorState)
{

	ImGui::Text("Simulation Delay (ms)");
	ImGui::SetItemTooltip("The delay between each step while the simulation is running.");
	
	ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 30.f);
	ImGui::SliderInt("##label test", &m_TickDelayMs, 0, 1000);
	ImGui::SetItemTooltip("Ctrl + Click to input value");
	ImGui::PopStyleVar();
	

	ImGui::Checkbox("Show Grid Lines", &m_GridLines);

	m_TickDelayMs = std::max(m_TickDelayMs, 0);
	return { .TickDelayMs = m_TickDelayMs, .GridLines = m_GridLines };
}
