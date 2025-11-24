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

gol::Size2F gol::StartButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x / 4.f, MultiActionButton::DefaultButtonHeight };  }
std::string gol::StartButton::Label(EditorState state) const
{
	switch (state.State)
	{
	using enum SimulationState;
	case Simulation: return ICON_FA_PAUSE;
	default:         return ICON_FA_PLAY;
	}
}
gol::GameAction gol::StartButton::Action(EditorState state) const
{
	switch (state.State)
	{
	using enum SimulationState;
	case Paint:      return GameAction::Start;
	case Paused:     return GameAction::Resume;
	case Simulation: return GameAction::Pause;
	}
	std::unreachable();
}
bool gol::StartButton::Enabled(EditorState state) const
{ 
	return state.State == SimulationState::Paint || state.State == SimulationState::Paused || state.State == SimulationState::Simulation; 
}

gol::Size2F     gol::ResetButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x / 3.f, ActionButton::DefaultButtonHeight }; }
std::string     gol::ResetButton::Label(EditorState) const { return ICON_FA_STOP; }
bool            gol::ResetButton::Enabled(EditorState state) const { return state.State == SimulationState::Simulation || state.State == SimulationState::Paused; }

gol::Size2F     gol::RestartButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x / 2.f, ActionButton::DefaultButtonHeight }; }
std::string     gol::RestartButton::Label(EditorState) const { return ICON_FA_REPEAT; }
bool            gol::RestartButton::Enabled(EditorState state) const { return state.State == SimulationState::Simulation || state.State == SimulationState::Paused; }

gol::Size2F     gol::ClearButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x, ActionButton::DefaultButtonHeight }; }
std::string     gol::ClearButton::Label(EditorState) const { return ICON_FA_TRASH; }
bool            gol::ClearButton::Enabled(EditorState state) const { return state.State != SimulationState::Empty; }

gol::SimulationControlResult gol::ExecutionWidget::Update(EditorState state)
{
	auto result = std::optional<GameAction> {};
	const auto updateIfNone = [&result](std::optional<GameAction> update)
	{
		if (!result)
			result = update;
	};

	updateIfNone(m_StartButton.Update(state));
	updateIfNone(m_ResetButton.Update(state));
	updateIfNone(m_RestartButton.Update(state));
	updateIfNone(m_ClearButton.Update(state));

	return { .Action = result };
}