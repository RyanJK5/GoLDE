#include "ExecutionWidget.h"

gol::SimulationControlResult gol::ExecutionWidget::Update(GameState state)
{
	auto result = GameAction::None;
	auto updateIfNone = [&result](GameAction update)
	{
		if (result == GameAction::None)
			result = update;
	};

	updateIfNone(m_StartButton.Update(state));
	updateIfNone(m_ClearButton.Update(state));
	updateIfNone(m_ResetButton.Update(state));
	updateIfNone(m_PauseButton.Update(state));
	updateIfNone(m_ResumeButton.Update(state));
	updateIfNone(m_RestartButton.Update(state));

	return { .Action = result };
}