#include "DelayWidget.h"

gol::SimulationControlResult gol::DelayWidget::Update(GameState)
{
	ImGui::Text("Simulation Delay (ms)");
	ImGui::SetItemTooltip("The delay between each step while the simulation is running.");
	ImGui::SliderInt("##label test", &m_TickDelayMs, 0, 1000);
	ImGui::SetItemTooltip("Ctrl + Click to input value");
	m_TickDelayMs = std::max(m_TickDelayMs, 0);
	return { .Action = GameAction::None, .TickDelayMs = m_TickDelayMs };
}
