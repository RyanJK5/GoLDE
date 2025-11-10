#include "EditorWidget.h"

gol::SimulationControlResult gol::EditorWidget::Update(GameState state)
{
	GameAction result = GameAction::None;
	const auto updateIfNone = [&result](GameAction update)
	{
		if (result == GameAction::None)
			result = update;
	};
	
	ImGui::Text("Editor");

	updateIfNone(m_CopyButton.Update(state));
	updateIfNone(m_CutButton.Update(state));
	if (result != GameAction::None)
		m_PasteButton.ClipboardCopied = true;
	updateIfNone(m_PasteButton.Update(state));
	
	return { .Action = result };
}
