#include "SimulationControl.h"

#include "GUILoader.h"
#include "Logging.h"

gol::SimulationControl::SimulationControl(const std::filesystem::path& shortcutsPath)
{
    auto fileInfo = StyleLoader::LoadYAML<ImVec4>(shortcutsPath);
    if (!fileInfo)
        throw std::exception(fileInfo.error().Description.c_str());
    CreateButtons(fileInfo->Shortcuts);
}

void gol::SimulationControl::FillResuts(SimulationControlResult& current, const SimulationControlResult& update) const
{
    if (current.Action == GameAction::None)
        current.Action = update.Action;
    if (!current.NewDimensions.has_value())
        current.NewDimensions = update.NewDimensions;
    if (!current.StepCount.has_value())
        current.StepCount = update.StepCount;
}

gol::SimulationControlResult gol::SimulationControl::Update(GameState state)
{
    ImGui::Begin("Simulation Control");

    ImGui::Text("Execution");

    SimulationControlResult result { .State = state };
    
    FillResuts(result, m_ExecutionWidget.Update(state));
    FillResuts(result, m_ResizeWidget.Update(state));
    FillResuts(result, m_StepWidget.Update(state));

    ImGui::End();
    return result;
}

void gol::SimulationControl::CreateButtons(const std::unordered_map<GameAction, std::vector<ImGuiKeyChord>>& shortcuts)
{
    m_ExecutionWidget = ExecutionWidget(shortcuts);
    m_ResizeWidget = ResizeWidget(shortcuts.at(GameAction::Resize));
    m_StepWidget = StepWidget(shortcuts.at(GameAction::Step));
}