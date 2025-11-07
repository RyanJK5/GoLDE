#ifndef __ExecutionWidget_h__
#define __ExecutionWidget_h__

#include "SimulationControlResult.h"
#include "GameActionButton.h"

namespace gol
{
    class StartButton : public TemplatedButton<"Start", GameAction::Start, true>
    {
    public:
        StartButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : TemplatedButtonInternal(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const override final { return { ImGui::GetContentRegionAvail().x / 3.f, GameActionButton::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const override final { return state == GameState::Paint; }
    };

    class ClearButton : public TemplatedButton<"Clear", GameAction::Clear, false>
    {
    public:
        ClearButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : TemplatedButtonInternal(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const override final { return { ImGui::GetContentRegionAvail().x / 2.f, GameActionButton::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const override final { return state != GameState::Empty; }
    };

    class ResetButton : public TemplatedButton<"Reset", GameAction::Reset, false>
    {
    public:
        ResetButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : TemplatedButtonInternal(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const override final { return { ImGui::GetContentRegionAvail().x, GameActionButton::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const override final { return state == GameState::Simulation || state == GameState::Paused; }
    };

    class PauseButton : public TemplatedButton<"Pause", GameAction::Pause, true>
    {
    public:
        PauseButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : TemplatedButtonInternal(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const override final { return { ImGui::GetContentRegionAvail().x / 3.f, GameActionButton::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const override final { return state == GameState::Simulation; }
    };

    class ResumeButton : public TemplatedButton<"Resume", GameAction::Resume, false>
    {
    public:
        ResumeButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : TemplatedButtonInternal(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const override final { return { ImGui::GetContentRegionAvail().x / 2.f, GameActionButton::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const override final { return state == GameState::Paused; }
    };

    class RestartButton : public TemplatedButton<"Restart", GameAction::Restart, false>
    {
    public:
        RestartButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : TemplatedButtonInternal(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const override final { return { ImGui::GetContentRegionAvail().x, GameActionButton::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const override final { return state == GameState::Simulation || state == GameState::Paused; }
    };

	class ExecutionWidget
	{
	public:
        ExecutionWidget() = default;

        ExecutionWidget(const std::unordered_map<GameAction, std::vector<ImGuiKeyChord>>& shortcuts)
            : m_StartButton   (shortcuts.at(GameAction::Start   ))
            , m_ClearButton   (shortcuts.at(GameAction::Clear   ))
            , m_ResetButton   (shortcuts.at(GameAction::Reset   ))
            , m_PauseButton   (shortcuts.at(GameAction::Pause   ))
            , m_ResumeButton  (shortcuts.at(GameAction::Resume  ))
            , m_RestartButton (shortcuts.at(GameAction::Restart ))
        { }

        SimulationControlResult Update(GameState state);
	private:
        StartButton m_StartButton;
        ClearButton m_ClearButton;
        ResetButton m_ResetButton;
        PauseButton m_PauseButton;
        ResumeButton m_ResumeButton;
        RestartButton m_RestartButton;
	};
}

#endif