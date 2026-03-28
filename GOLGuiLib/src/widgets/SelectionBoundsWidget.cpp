#include "SelectionBoundsWidget.hpp"

#include <array>

namespace gol {
    WidgetResult SelectionBoundsWidget::UpdateImpl(const EditorResult& state) {
        
        const auto inputBounds =
            state.SelectionBounds ? state.SelectionBounds : Rect{};
        
        std::array data{inputBounds->X, inputBounds->Y,
                        inputBounds->Width, inputBounds->Height};
        
        ImGui::Text("Selection Bounds");

        ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 30.f);
        
        ImGui::InputInt4("##SelectionBoundsLabel", data.data());
        const bool copyBounds = ImGui::IsItemDeactivatedAfterEdit();

        ImGui::Separator();
        ImGui::PopStyleVar();

        if (copyBounds) {
            const Rect result{data[0], data[1], data[2], data[3]};
            if (result != Rect{} && state.SelectionBounds != result) {
                return {.Command = SelectionBoundsCommand{.Bounds = result}};
            }
        }

        return {};
    }
}