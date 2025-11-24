#include <algorithm>
#include <imgui/imgui.h>

#include "DelayWidget.h"
#include "GameEnums.h"
#include "SimulationControlResult.h"

gol::SimulationControlResult gol::DelayWidget::Update(EditorState)
{
	ImGui::Text("Simulation Delay (ms)");
	ImGui::SetItemTooltip("The delay between each step while the simulation is running.");
	ImGui::SliderInt("##label test", &m_TickDelayMs, 0, 1000);
	ImGui::SetItemTooltip("Ctrl + Click to input value");
	m_TickDelayMs = std::max(m_TickDelayMs, 0);
	return { .TickDelayMs = m_TickDelayMs };
}
