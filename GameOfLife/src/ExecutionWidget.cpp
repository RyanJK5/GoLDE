#include <imgui/imgui.h>

#include "ExecutionWidget.h"
#include "GameEnums.h"
#include "SimulationControlResult.h"

gol::SimulationControlResult gol::ExecutionWidget::Update(GameState state)
{
	auto result = GameAction::None;
	const auto updateIfNone = [&result](GameAction update)
	{
		if (result == GameAction::None)
			result = update;
	};

	ImGui::Text("Execution");
	updateIfNone(m_StartButton.Update(state));
	updateIfNone(m_ClearButton.Update(state));
	updateIfNone(m_ResetButton.Update(state));
	updateIfNone(m_PauseButton.Update(state));
	updateIfNone(m_ResumeButton.Update(state));
	updateIfNone(m_RestartButton.Update(state));

	return { .Action = result };
}