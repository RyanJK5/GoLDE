#include <imgui/imgui.h>
#include <vector>

#include "ConfigLoader.h"
#include "KeyShortcut.h"
#include "GameEnums.h"
#include "SimulationControl.h"
#include "SimulationControlResult.h"

gol::SelectionShortcuts::SelectionShortcuts(
    const std::vector<ImGuiKeyChord>& left,
    const std::vector<ImGuiKeyChord>& right,
    const std::vector<ImGuiKeyChord>& up,
    const std::vector<ImGuiKeyChord>& down
)
    : Shortcuts({
        { SelectionAction::NudgeLeft,  left | KeyShortcut::MapChordsToVector },
        { SelectionAction::NudgeRight, right | KeyShortcut::MapChordsToVector },
        { SelectionAction::NudgeUp,    up | KeyShortcut::MapChordsToVector },
        { SelectionAction::NudgeDown,  down | KeyShortcut::MapChordsToVector }
    })
{ }

gol::SimulationControlResult gol::SelectionShortcuts::Update(const EditorResult& state)
{
	SimulationControlResult result { .NudgeSize = 1 };
    for (auto&& [action, actionShortcuts] : Shortcuts)
    {
        if (action != SelectionAction::Deselect && state.State != SimulationState::Paint)
            continue;

        bool resultActive = false;
        for (auto& shortcut : actionShortcuts)
        {
            bool shortcutActive = shortcut.Active();
            if (shortcutActive && ((shortcut.Shortcut() & ImGuiMod_Shift) != 0))
                result.NudgeSize = 10;
            resultActive = shortcutActive || resultActive;
        }
        if (resultActive && !result.Action)
            result.Action = action;
    }
    if (result.Action)
		result.FromShortcut = true;
    return result;
}

gol::SimulationControl::SimulationControl(const StyleLoader::StyleInfo<ImVec4>& fileInfo)
    : m_SelectionShortcuts(
        fileInfo.Shortcuts.at(SelectionAction::NudgeLeft),
        fileInfo.Shortcuts.at(SelectionAction::NudgeRight),
        fileInfo.Shortcuts.at(SelectionAction::NudgeUp),
        fileInfo.Shortcuts.at(SelectionAction::NudgeDown)
    )
    , m_ExecutionWidget(fileInfo.Shortcuts)
    , m_EditorWidget(fileInfo.Shortcuts)
    , m_FileWidget(fileInfo.Shortcuts.at(EditorAction::UpdateFile), fileInfo.Shortcuts.at(EditorAction::Save), fileInfo.Shortcuts.at(EditorAction::Load))
    , m_ResizeWidget(fileInfo.Shortcuts.at(EditorAction::Resize))
    , m_StepWidget(fileInfo.Shortcuts.at(GameAction::Step))
    , m_DelayWidget()
{ }

void gol::SimulationControl::FillResults(SimulationControlResult& current, const SimulationControlResult& update) const
{
    if (!current.Action)
        current.Action = update.Action;
    if (!current.NewDimensions)
        current.NewDimensions = update.NewDimensions;
    if (!current.StepCount)
        current.StepCount = update.StepCount;
    if (!current.TickDelayMs)
        current.TickDelayMs = update.TickDelayMs;
    if (current.NudgeSize == 0)
		current.NudgeSize = update.NudgeSize;
    if (!current.FilePath)
		current.FilePath = update.FilePath;
    if (!current.GridLines)
        current.GridLines = update.GridLines;
    if (!current.FromShortcut)
		current.FromShortcut = update.FromShortcut;
}

gol::SimulationControlResult gol::SimulationControl::Update(const EditorResult& state)
{
    ImGui::Begin("Simulation Control", nullptr, ImGuiWindowFlags_NoNavInputs);

    SimulationControlResult result { .State = state.State };
    
    FillResults(result, m_ExecutionWidget.Update(state));
    FillResults(result, m_EditorWidget.Update(state));
    FillResults(result, m_FileWidget.Update(state));
    FillResults(result, m_StepWidget.Update(state));
    FillResults(result, m_ResizeWidget.Update(state));
    FillResults(result, m_DelayWidget.Update(state));
    FillResults(result, m_SelectionShortcuts.Update(state));

    ImGui::End();
    return result;
}