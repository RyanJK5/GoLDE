#ifndef __SimulationControlButtons_h__
#define __SimulationControlButtons_h__

#include "GameActionButton.h"

namespace gol
{
    class StartButton : public TemplatedButton<"Start", GameAction::Start, true>
    {
    public:
        StartButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : HiddenTemplatedButton(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x / 3.f, GameActionButton::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const { return state == GameState::Paint; }
    };

    class ClearButton : public TemplatedButton<"Clear", GameAction::Clear, false>
    {
    public:
        ClearButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : HiddenTemplatedButton(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x / 2.f, GameActionButton::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const final { return state != GameState::Empty; }
    };

    class ResetButton : public TemplatedButton<"Reset", GameAction::Reset, false>
    {
    public:
        ResetButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : HiddenTemplatedButton(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x, GameActionButton::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const final { return state == GameState::Simulation || state == GameState::Paused; }
    };

    class PauseButton : public TemplatedButton<"Pause", GameAction::Pause, true>
    {
    public:
        PauseButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : HiddenTemplatedButton(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x / 3.f, GameActionButton::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const final { return state == GameState::Simulation; }
    };

    class ResumeButton : public TemplatedButton<"Resume", GameAction::Resume, false>
    {
    public:
        ResumeButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : HiddenTemplatedButton(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x / 2.f, GameActionButton::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const final { return state == GameState::Paused; }
    };

    class RestartButton : public TemplatedButton<"Restart", GameAction::Restart, false>
    {
    public:
        RestartButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : HiddenTemplatedButton(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x, GameActionButton::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const final { return state == GameState::Simulation || state == GameState::Paused; }
    };

    class StepButton : public TemplatedButton<"Step", GameAction::Step, false>
    {
    public:
        StepButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : HiddenTemplatedButton(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x, GameActionButton::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const final { return state == GameState::Paint || state == GameState::Paused; }
    };

#endif
}