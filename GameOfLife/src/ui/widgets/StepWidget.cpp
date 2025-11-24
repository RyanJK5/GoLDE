#include <imgui/imgui.h>
#include <utility>

#include "GameEnums.h"
#include "SimulationControlResult.h"
#include "StepWidget.h"

gol::SimulationControlResult gol::StepWidget::Update(EditorState state)
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

    ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 30.f);
    auto action = m_Button.Update(state);
    ImGui::Separator();
    ImGui::PopStyleVar();

    return 
    {
        .Action = action,
        .StepCount = m_StepCount
    };
}