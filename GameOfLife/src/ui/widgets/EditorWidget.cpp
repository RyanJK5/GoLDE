#include <optional>
#include <imgui/imgui.h>

#include "EditorWidget.h"
#include "GameEnums.h"
#include "SimulationControlResult.h"

gol::SimulationControlResult gol::EditorWidget::UpdateImpl(const EditorResult& state)
{
	auto result = SimulationControlResult {};
	
	UpdateResult(result, m_CopyButton.Update(state));
	UpdateResult(result, m_PasteButton.Update(state));
	UpdateResult(result, m_CutButton.Update(state));
	UpdateResult(result, m_DeleteButton.Update(state));
				 
	UpdateResult(result, m_DeselectButton.Update(state));
	UpdateResult(result, m_RotateButton.Update(state));
	UpdateResult(result, m_UndoButton.Update(state));
	UpdateResult(result, m_RedoButton.Update(state));
	
	return result;
}
