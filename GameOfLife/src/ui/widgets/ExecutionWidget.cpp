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
std::string gol::StartButton::Label(SimulationState state) const 
{
	switch (state)
	{
	using enum SimulationState;
	case Simulation: return ICON_FA_PAUSE;
	default:         return ICON_FA_PLAY;
	}
}
gol::GameAction gol::StartButton::Action(SimulationState state) const 
{
	switch (state)
	{
	using enum SimulationState;
	case Paint:      return GameAction::Start;
	case Paused:     return GameAction::Resume;
	case Simulation: return GameAction::Pause;
	}
	std::unreachable();
}
bool gol::StartButton::Enabled(SimulationState state) const 
{ 
	return state == SimulationState::Paint || state == SimulationState::Paused || state == SimulationState::Simulation; 
}

gol::Size2F     gol::ResetButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x / 3.f, ActionButton::DefaultButtonHeight }; }
std::string     gol::ResetButton::Label(SimulationState) const { return ICON_FA_STOP; }
bool            gol::ResetButton::Enabled(SimulationState state) const { return state == SimulationState::Simulation || state == SimulationState::Paused; }

gol::Size2F     gol::RestartButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x / 2.f, ActionButton::DefaultButtonHeight }; }
std::string     gol::RestartButton::Label(SimulationState) const { return ICON_FA_REPEAT; }
bool            gol::RestartButton::Enabled(SimulationState state) const { return state == SimulationState::Simulation || state == SimulationState::Paused; }

gol::Size2F     gol::ClearButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x, ActionButton::DefaultButtonHeight }; }
std::string     gol::ClearButton::Label(SimulationState) const { return ICON_FA_TRASH; }
bool            gol::ClearButton::Enabled(SimulationState state) const { return state != SimulationState::Empty; }

gol::SimulationControlResult gol::ExecutionWidget::Update(SimulationState state)
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