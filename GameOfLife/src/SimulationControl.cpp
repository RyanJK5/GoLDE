#include "SimulationControl.h"
#include "GUILoader.h"
#include "Logging.h"

gol::SimulationControl::SimulationControl(const std::filesystem::path& shortcutsPath)
{
    auto fileInfo = StyleLoader::LoadYAML<ImVec4>(shortcutsPath);
    if (!fileInfo)
        throw std::exception(fileInfo.error().Description.data());
    CreateButtons(fileInfo->Shortcuts);
}

gol::SimulationControlResult gol::SimulationControl::Update(GameState state)
{
    ImGui::Begin("Simulation Control");

    GameAction result = GameAction::None;
    for (auto& button : m_BasicButtons)
    {
        GameAction action = button->Update(state);
        if (result == GameAction::None)
            result = action;
    }

    GameAction action = UpdateStepButton(state);
    if (result == GameAction::None)
        result = action;

    ImGui::End();
    return { state, result, m_StepCount };
}

gol::GameAction gol::SimulationControl::UpdateStepButton(GameState state)
{
    ImGui::PushStyleVarY(ImGuiStyleVar_FramePadding, 10.f);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x / 3.f * 2.f + 5);
    ImGui::InputInt("##label", &m_StepCount, 1);
    ImGui::PopStyleVar();

    if (m_StepCount < 1)
        m_StepCount = 1;
    if (m_StepCount >= StepWarning)
        ImGui::SetItemTooltip("Stepping with large values may cause lag!");

    return m_Step.Update(state);
}

void gol::SimulationControl::CreateButtons(const std::unordered_map<GameAction, std::vector<ImGuiKeyChord>>& shortcuts)
{
    m_BasicButtons.push_back(std::make_unique<StartButton>   ( shortcuts.at(GameAction::Start   )));
    m_BasicButtons.push_back(std::make_unique<ClearButton>   ( shortcuts.at(GameAction::Clear   )));
    m_BasicButtons.push_back(std::make_unique<ResetButton>   ( shortcuts.at(GameAction::Reset   )));
    m_BasicButtons.push_back(std::make_unique<PauseButton>   ( shortcuts.at(GameAction::Pause   )));
    m_BasicButtons.push_back(std::make_unique<ResumeButton>  ( shortcuts.at(GameAction::Resume  )));
    m_BasicButtons.push_back(std::make_unique<RestartButton> ( shortcuts.at(GameAction::Restart )));
    
    m_Step = StepButton { shortcuts.at(GameAction::Step) };
}