#include <optional>
#include <imgui/imgui.h>

#include "EditorWidget.h"
#include "GameEnums.h"
#include "SimulationControlResult.h"

gol::SimulationControlResult gol::EditorWidget::Update(EditorState state)
{
	auto result = std::optional<ActionVariant> {};
	const auto updateIfNone = [&result](std::optional<ActionVariant> update)
	{
		if (!result)
			result = update;
	};
	
	updateIfNone(m_CopyButton.Update(state));
	updateIfNone(m_PasteButton.Update(state));
	updateIfNone(m_CutButton.Update(state));
	updateIfNone(m_DeleteButton.Update(state));

	updateIfNone(m_DeselectButton.Update(state));
	updateIfNone(m_RotateButton.Update(state));
	updateIfNone(m_UndoButton.Update(state));
	ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 30.f);
	updateIfNone(m_RedoButton.Update(state));
	
	ImGui::Separator();
	ImGui::PopStyleVar();

	return { .Action = result };
}
