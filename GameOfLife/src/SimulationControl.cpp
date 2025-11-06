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
        GameAction action = button.Update(state);
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
        ImGui::SetItemTooltip("Caution: Stepping with large values may cause lag!");

    return m_Step.Update(state);
}

void gol::SimulationControl::CreateButtons(const std::unordered_map<GameAction, std::vector<ImGuiKeyChord>>& shortcuts)
{
    const Size2F size = { 100, 50 };
    m_BasicButtons[0] = {
        "Start",
        GameAction::Start,
        [size]() { return Size2F { ImGui::GetContentRegionAvail().x / 3.f, size.Height }; },
        [](auto state) { return state == GameState::Paint; },
        shortcuts.at(GameAction::Start),
        true
    };
    m_BasicButtons[1] = {
        "Clear",
        GameAction::Clear,
        [size]() { return Size2F { ImGui::GetContentRegionAvail().x / 2.f, size.Height }; },
        [](auto state) { return state != GameState::Empty; },
        shortcuts.at(GameAction::Clear)
    };
    m_BasicButtons[2] = {
        "Reset",
        GameAction::Reset,
        [size]() { return Size2F { ImGui::GetContentRegionAvail().x, size.Height }; },
        [](auto state) { return state == GameState::Simulation || state == GameState::Paused; },
        shortcuts.at(GameAction::Reset)
    };
    m_BasicButtons[3] = {
        "Pause",
        GameAction::Pause,
        [size]() { return Size2F { ImGui::GetContentRegionAvail().x / 3.f, size.Height }; },
        [](auto state) { return state == GameState::Simulation; },
        shortcuts.at(GameAction::Pause),
        true
    };
    m_BasicButtons[4] = {
        "Resume",
        GameAction::Resume,
        [size]() { return Size2F { ImGui::GetContentRegionAvail().x / 2.f, size.Height }; },
        [](auto state) { return state == GameState::Paused; },
        shortcuts.at(GameAction::Resume)
    };
    m_BasicButtons[5] = {
        "Restart",
        GameAction::Restart,
        [size]() { return Size2F { ImGui::GetContentRegionAvail().x, size.Height }; },
        [](auto state) { return state == GameState::Simulation || state == GameState::Paused; },
        shortcuts.at(GameAction::Restart)
    };

    m_Step = {
        "Step",
        GameAction::Step,
        [size]() { return Size2F { ImGui::GetContentRegionAvail().x, size.Height }; },
        [](auto state) { return state == GameState::Paint || state == GameState::Paused; },
        shortcuts.at(GameAction::Step)
    };
}
