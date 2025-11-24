#include <cstdint>
#include <imgui/imgui.h>

#include "GameEnums.h"
#include "ResizeWidget.h"
#include "SimulationControlResult.h"

gol::SimulationControlResult gol::ResizeWidget::Update(EditorState state)
{
    const float totalWidth = ImGui::GetContentRegionAvail().x;
    ImGui::SetCursorPosX(ImGui::GetStyle().FramePadding.x * 3);
    ImGui::Text("Width");
    ImGui::SameLine();
    ImGui::SetCursorPosX(totalWidth / 3.f + ImGui::GetStyle().FramePadding.x * 3);
    ImGui::Text("Height");

    ImGui::PushStyleVarY(ImGuiStyleVar_FramePadding, 10.f);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x / 3.f * 2.f + 5);

    int32_t wrapper[2] = { m_Dimensions.Width, m_Dimensions.Height };
    ImGui::InputInt2("##label", wrapper);
    m_Dimensions = { wrapper[0], wrapper[1] };

    ImGui::PopStyleVar();

    ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 30.f);

    auto action = m_Button.Update(state);
	if (action == EditorAction::Resize && (m_Dimensions.Width == 0 || m_Dimensions.Height == 0))
    {
        m_Dimensions.Width = 0;
        m_Dimensions.Height = 0;
    }

    ImGui::Separator();
    ImGui::PopStyleVar();

    return 
    {
        .Action = action,
        .NewDimensions = m_Dimensions
    };
}