#include <font-awesome/IconsFontAwesome7.h>
#include <imgui/imgui.h>
#include <optional>
#include <string>
#include <utility>

#include "ActionButton.h"
#include "ExecutionWidget.h"
#include "GameEnums.h"
#include "Graphics2D.h"
#include "SimulationControlResult.h"

gol::Size2F gol::StartButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x / 3.f, MultiActionButton::DefaultButtonHeight };  }
std::string gol::StartButton::Label(GameState state) const 
{
	switch (state)
	{
	using enum GameState;
	case Simulation: return ICON_FA_PAUSE;
	default:         return ICON_FA_PLAY;
	}
}
gol::GameAction gol::StartButton::Action(GameState state) const 
{
	switch (state)
	{
	using enum GameState;
	case Paint:      return GameAction::Start;
	case Paused:     return GameAction::Resume;
	case Simulation: return GameAction::Pause;
	}
	std::unreachable();
}
bool gol::StartButton::Enabled(GameState state) const 
{ 
	return state == GameState::Paint || state == GameState::Paused || state == GameState::Simulation; 
}

gol::Size2F     gol::ClearButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x / 2.f, ActionButton::DefaultButtonHeight }; }
std::string     gol::ClearButton::Label(GameState) const { return ICON_FA_RETWEET; }
bool            gol::ClearButton::Enabled(GameState state) const { return state != GameState::Empty; }

gol::Size2F     gol::ResetButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x, ActionButton::DefaultButtonHeight }; }
std::string     gol::ResetButton::Label(GameState) const { return ICON_FA_STOP; }
bool            gol::ResetButton::Enabled(GameState state) const { return state == GameState::Simulation || state == GameState::Paused; }

gol::Size2F     gol::RestartButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x, ActionButton::DefaultButtonHeight }; }
std::string     gol::RestartButton::Label(GameState) const { return ICON_FA_REPEAT; }
bool            gol::RestartButton::Enabled(GameState state) const { return state == GameState::Simulation || state == GameState::Paused; }

gol::SimulationControlResult gol::ExecutionWidget::Update(GameState state)
{
	auto result = std::optional<GameAction> {};
	const auto updateIfNone = [&result](std::optional<GameAction> update)
	{
		if (!result)
			result = update;
	};

	ImGui::Text("Execution");
	updateIfNone(m_StartButton.Update(state));
	updateIfNone(m_ClearButton.Update(state));
	updateIfNone(m_ResetButton.Update(state));
	updateIfNone(m_RestartButton.Update(state));

	return { .Action = result };
}