#include <imgui/imgui.h>

#include "GameEnums.h"
#include "GUILoader.h"
#include "SimulationControl.h"
#include "SimulationControlResult.h"

gol::SimulationControlResult gol::SelectionShortcuts::Update(GameState state)
{
	SimulationControlResult result { .Action = GameAction::None, .NudgeSize = 1 };

    for (auto&& [action, actionShortcuts] : Shortcuts)
    {
        if (action != GameAction::Deselect && state != GameState::Paint)
            continue;

        bool resultActive = false;
        for (auto& shortcut : actionShortcuts)
        {
            bool shortcutActive = shortcut.Active();
            if (shortcutActive && ((shortcut.Shortcut() & ImGuiMod_Shift) != 0))
                result.NudgeSize = 10;
            resultActive = shortcutActive || resultActive;
        }
        if (resultActive && result.Action == GameAction::None)
            result.Action = action;
    }
    return result;
}

gol::SimulationControl::SimulationControl(const StyleLoader::StyleInfo<ImVec4>& fileInfo)
    : m_SelectionShortcuts(
        fileInfo.Shortcuts.at(GameAction::NudgeLeft),
        fileInfo.Shortcuts.at(GameAction::NudgeRight),
        fileInfo.Shortcuts.at(GameAction::NudgeUp),
        fileInfo.Shortcuts.at(GameAction::NudgeDown),
        fileInfo.Shortcuts.at(GameAction::Deselect),
        fileInfo.Shortcuts.at(GameAction::Rotate)
    )
    , m_VersionManager(
        fileInfo.Shortcuts.at(GameAction::Undo),
        fileInfo.Shortcuts.at(GameAction::Redo)
    )
    , m_ExecutionWidget(fileInfo.Shortcuts)
    , m_ResizeWidget(fileInfo.Shortcuts.at(GameAction::Resize))
    , m_StepWidget(fileInfo.Shortcuts.at(GameAction::Step))
    , m_DelayWidget()
    , m_EditorWidget(fileInfo.Shortcuts)
{ }

void gol::SimulationControl::FillResults(SimulationControlResult& current, const SimulationControlResult& update) const
{
    if (current.Action == GameAction::None)
        current.Action = update.Action;
    if (!current.NewDimensions)
        current.NewDimensions = update.NewDimensions;
    if (!current.StepCount)
        current.StepCount = update.StepCount;
    if (!current.TickDelayMs)
        current.TickDelayMs = update.TickDelayMs;
    if (current.NudgeSize == 0)
		current.NudgeSize = update.NudgeSize;
}

gol::SimulationControlResult gol::SimulationControl::Update(GameState state)
{
    ImGui::Begin("Simulation Control");

    SimulationControlResult result { .State = state };
    
    FillResults(result, m_VersionManager.Update(state));
    FillResults(result, m_ExecutionWidget.Update(state));
    FillResults(result, m_ResizeWidget.Update(state));
    FillResults(result, m_StepWidget.Update(state));
    FillResults(result, m_DelayWidget.Update(state));
    FillResults(result, m_EditorWidget.Update(state));
    FillResults(result, m_SelectionShortcuts.Update(state));

    ImGui::End();
    return result;
}