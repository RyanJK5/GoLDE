#include "RuleWidget.hpp"
#include "DisabledScope.hpp"
#include "LifeRule.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>

namespace gol {
RuleWidget::RuleWidget()
    : m_InputError("Invalid Rule",
                   [this](auto) { m_InputText = m_LastValid; }) {}

WidgetResult RuleWidget::UpdateImpl(const EditorResult& state) {
    ImGui::PushStyleVarY(ImGuiStyleVar_FramePadding, 10.f);
    ImGui::Text("Rule");

    const bool pressedEnter = ImGui::InputTextWithHint(
        "##RuleWidgetInput", "Enter Rule...", &m_InputText,
        ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SetItemTooltip(
        "Enter any outer-totalistic rule, which is defined as "
        "\"B1...8/S0...8\". The digits after 'B'\n"
        "specify the number of neighbors a cell can have to be born, and the "
        "digits after 'S'\n"
        "specify the number of neighbors a cell can have to survive to the "
        "next generation.\n"
        "Conway's Game of Life is therefore defined as B3/S23.");

    ImGui::SameLine();
    ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 30.f);

    const bool pressedButton = [&] {
        DisabledScope disableIf{!Actions::Editable(state.Simulation.State)};
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        return ImGui::Button("Set Rule");
    }();

    ImGui::Separator();
    ImGui::PopStyleVar();

    ImGui::PopStyleVar();

    m_InputError.Update();

    if (!Actions::Editable(state.Simulation.State) ||
        (!pressedEnter && !pressedButton)) {
        return {};
    }

    const auto validRule = LifeRule::IsValidRule(m_InputText);
    if (!validRule) {
        m_InputError.Message = validRule.error();
        m_InputError.Activate();
        return {};
    }

    m_LastValid = m_InputText;
    return WidgetResult{.Command = RuleCommand{m_InputText}};
}
} // namespace gol