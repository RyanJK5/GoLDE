#include "CameraPositionWidget.hpp"

#include <array>

namespace gol {
WidgetResult CameraPositionWidget::UpdateImpl(const EditorResult&) {
    std::array data{m_Position.X, m_Position.Y};

    ImGui::Text("Camera Position");

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x / 3.f * 2.f + 5);
    ImGui::PushStyleVarY(ImGuiStyleVar_FramePadding, 10.f);

    ImGui::InputInt2("##CameraPositionLabel", data.data());
    m_Position.X = data[0];
    m_Position.Y = data[1];

    ImGui::SameLine();

    ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 30.f);

    auto ret = [&] {
        if (ImGui::Button("Travel", {ImGui::GetContentRegionAvail().x, 0})) {
            return WidgetResult{.Command = CameraPositionCommand{m_Position}};
        }
        return WidgetResult{};
    }();

    ImGui::SetItemTooltip("Move the camera to the specified position.");
    ImGui::Separator();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
    return ret;
}
} // namespace gol