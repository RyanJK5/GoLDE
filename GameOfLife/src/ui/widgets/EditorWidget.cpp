#include <optional>
#include <imgui/imgui.h>

#include "EditorWidget.h"
#include "GameEnums.h"
#include "SimulationControlResult.h"

gol::SimulationControlResult gol::EditorWidget::Update(SimulationState state)
{
	auto result = std::optional<SelectionAction> {};
	const auto updateIfNone = [&result](std::optional<SelectionAction> update)
	{
		if (!result)
			result = update;
	};
	
	updateIfNone(m_CopyButton.Update(state));
	updateIfNone(m_CutButton.Update(state));
	updateIfNone(m_PasteButton.Update(state));
	ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 30.f);
	updateIfNone(m_DeleteButton.Update(state));
	
	ImGui::Separator();
	ImGui::PopStyleVar();

	return { .Action = result };
}
