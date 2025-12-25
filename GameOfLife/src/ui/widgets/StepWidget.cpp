#include <font-awesome/IconsFontAwesome7.h>
#include <imgui/imgui.h>
#include <string>
#include <utility>

#include "GameEnums.h"
#include "Graphics2D.h"
#include "SimulationControlResult.h"
#include "StepWidget.h"

gol::Size2F gol::StepButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x, ActionButton::DefaultButtonHeight }; }
std::string gol::StepButton::Label(const EditorResult&) const { return ICON_FA_FORWARD_STEP; }
bool        gol::StepButton::Enabled(const EditorResult& state) const { return state.State == SimulationState::Paint || state.State == SimulationState::Paused; }

gol::SimulationControlResult gol::StepWidget::UpdateImpl(const EditorResult& state)
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
    auto result = m_Button.Update(state);
    ImGui::Separator();
    ImGui::PopStyleVar();

    return 
    {
        .Action = result.Action,
        .StepCount = m_StepCount,
        .FromShortcut = result.FromShortcut
    };
}