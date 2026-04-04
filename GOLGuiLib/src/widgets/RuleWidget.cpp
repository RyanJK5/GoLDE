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
    bool success = ImGui::InputTextWithHint(
        "##RuleWidgetInput", "Enter Rule...", &m_InputText,
        ImGuiInputTextFlags_EnterReturnsTrue);

    ImGui::SameLine();
    ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 30.f);

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

    {
        DisabledScope disableIf{!Actions::Editable(state.Simulation.State)};
        success = ImGui::Button("Set Rule") || success;
    }

    ImGui::Separator();
    ImGui::PopStyleVar();

    ImGui::PopStyleVar();

    m_InputError.Update();

    if (!success) {
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