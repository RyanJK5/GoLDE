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

gol::SimulationControlResult gol::SimulationControl::Update(GameState state)
{
    ImGui::Begin("Simulation Control");

    ImGui::Text("Execution");
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

    action = UpdateDimensions(state);
    if (result == GameAction::None)
        result = action;

    ImGui::End();
    return 
    { 
        .State = state, 
        .Action = result, 
        .StepCount = m_StepCount, 
        .NewDimensions = m_Dimensions 
    };
}

gol::GameAction gol::SimulationControl::UpdateStepButton(GameState state)
{
    ImGui::Text("Step Count");

    ImGui::PushStyleVarY(ImGuiStyleVar_FramePadding, 10.f);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x / 3.f * 2.f + 5);
    ImGui::InputInt("##label", &m_StepCount, 1, BigStep);
    ImGui::PopStyleVar();

    if (m_StepCount < 1)
        m_StepCount = 1;
    if (m_StepCount >= StepWarning)
        ImGui::SetItemTooltip("Stepping with large values may cause lag!");
    
    return m_StepButton.Update(state);
}

gol::GameAction gol::SimulationControl::UpdateDimensions(GameState state)
{
    const float totalWidth = ImGui::GetContentRegionAvail().x;
    ImGui::SetCursorPosX(ImGui::GetStyle().FramePadding.x * 3);
    ImGui::Text("Width");
    ImGui::SameLine();
    ImGui::SetCursorPosX(totalWidth / 3.f + ImGui::GetStyle().FramePadding.x * 3);
    ImGui::Text("Height");


    ImGui::PushStyleVarY(ImGuiStyleVar_FramePadding, 10.f);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x / 3.f * 2.f + 5);

    int32_t wrapper[2] = { m_Dimensions.Width, m_Dimensions.Height };
    
    ImGui::InputInt2("##label", wrapper);
    m_Dimensions = { wrapper[0], wrapper[1] };

    ImGui::PopStyleVar();

    auto result = GameAction::None;
    auto action = m_ResizeButton.Update(state);
    if (result == GameAction::None)
        result = action;

    return result;
}

void gol::SimulationControl::CreateButtons(const std::unordered_map<GameAction, std::vector<ImGuiKeyChord>>& shortcuts)
{
    m_BasicButtons.push_back(std::make_unique<StartButton>   ( shortcuts.at(GameAction::Start   )));
    m_BasicButtons.push_back(std::make_unique<ClearButton>   ( shortcuts.at(GameAction::Clear   )));
    m_BasicButtons.push_back(std::make_unique<ResetButton>   ( shortcuts.at(GameAction::Reset   )));
    m_BasicButtons.push_back(std::make_unique<PauseButton>   ( shortcuts.at(GameAction::Pause   )));
    m_BasicButtons.push_back(std::make_unique<ResumeButton>  ( shortcuts.at(GameAction::Resume  )));
    m_BasicButtons.push_back(std::make_unique<RestartButton> ( shortcuts.at(GameAction::Restart )));
    
    m_StepButton = StepButton     { shortcuts.at(GameAction::Step   )};
    m_ResizeButton = ResizeButton { shortcuts.at(GameAction::Resize )};
}