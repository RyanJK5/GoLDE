#include <imgui/imgui.h>
#include <utility>

#include "GameEnums.h"
#include "SimulationControlResult.h"
#include "StepWidget.h"

gol::SimulationControlResult gol::StepWidget::Update(GameState state)
{
    ImGui::Text("Step Count");

    ImGui::PushStyleVarY(ImGuiStyleVar_FramePadding, 10.f);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x / 3.f * 2.f + 5);
    ImGui::InputInt("##label", &m_StepCount, 1, BigStep);
    ImGui::PopStyleVar();

    if (m_StepCount < 1)
        m_StepCount = 1;
    if (m_StepCount >= StepWarning)
        ImGui::SetItemTooltip("Stepping with large values may cause lag!");

    SimulationControlResult result;
    result.Action = m_Button.Update(state);
    result.StepCount = m_StepCount;
    return std::move(result);
}