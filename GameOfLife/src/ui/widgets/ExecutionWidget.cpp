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
std::string gol::StartButton::Label(const EditorResult& state) const
{
	switch (state.State)
	{
	using enum SimulationState;
	case Simulation: return ICON_FA_PAUSE;
	default:         return ICON_FA_PLAY;
	}
}
gol::GameAction gol::StartButton::Action(const EditorResult& state) const
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
bool gol::StartButton::Enabled(const EditorResult& state) const
{ 
	return state.State == SimulationState::Paint || state.State == SimulationState::Paused || state.State == SimulationState::Simulation; 
}

gol::Size2F     gol::ResetButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x / 3.f, ActionButton::DefaultButtonHeight }; }
std::string     gol::ResetButton::Label(const EditorResult&) const { return ICON_FA_STOP; }
bool            gol::ResetButton::Enabled(const EditorResult& state) const { return state.State == SimulationState::Simulation || state.State == SimulationState::Paused; }

gol::Size2F     gol::RestartButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x / 2.f, ActionButton::DefaultButtonHeight }; }
std::string     gol::RestartButton::Label(const EditorResult&) const { return ICON_FA_REPEAT; }
bool            gol::RestartButton::Enabled(const EditorResult& state) const { return state.State == SimulationState::Simulation || state.State == SimulationState::Paused; }

gol::Size2F     gol::ClearButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x, ActionButton::DefaultButtonHeight }; }
std::string     gol::ClearButton::Label(const EditorResult&) const { return ICON_FA_TRASH; }
bool            gol::ClearButton::Enabled(const EditorResult& state) const { return state.State != SimulationState::Empty; }

gol::SimulationControlResult gol::ExecutionWidget::UpdateImpl(const EditorResult& state)
{
	auto result = SimulationControlResult {};
	UpdateResult(result, m_StartButton.Update(state));
	UpdateResult(result, m_ResetButton.Update(state));
	UpdateResult(result, m_RestartButton.Update(state));
	UpdateResult(result, m_ClearButton.Update(state));

	return result;
}