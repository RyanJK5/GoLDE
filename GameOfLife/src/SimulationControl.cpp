#include "SimulationControl.h"

#include "Logging.h"

gol::SimulationControl::SimulationControl(const StyleLoader::StyleInfo<ImVec4>& fileInfo)
    : m_ExecutionWidget(fileInfo.Shortcuts)
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
}

gol::SimulationControlResult gol::SimulationControl::Update(GameState state)
{
    ImGui::Begin("Simulation Control");

    SimulationControlResult result { .State = state };
    
    FillResults(result, m_ExecutionWidget.Update(state));
    FillResults(result, m_ResizeWidget.Update(state));
    FillResults(result, m_StepWidget.Update(state));
    FillResults(result, m_DelayWidget.Update(state));
    FillResults(result, m_EditorWidget.Update(state));

    ImGui::End();
    return result;
}