#ifndef __EditorWidget_h__
#define __EditorWidget_h__

#include <imgui/imgui.h>
#include <span>
#include <unordered_map>
#include <vector>

#include "GameActionButton.h"
#include "GameEnums.h"
#include "Graphics2D.h"
#include "SimulationControlResult.h"


namespace gol
{
    class CopyButton : public TemplatedButton<"Copy", GameAction::Copy, true>
    {
    public:
        CopyButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : TemplatedButtonInternal(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x / 4.f, GameActionButton::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const final { return state == GameState::Paint || state == GameState::Empty || state == GameState::Paused; }
    };

    class CutButton : public TemplatedButton<"Cut", GameAction::Cut, false>
    {
    public:
        CutButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : TemplatedButtonInternal(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x / 3.f, GameActionButton::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const final { return state == GameState::Paint || state == GameState::Empty; }
    };

    class PasteButton : public TemplatedButton<"Paste", GameAction::Paste, false>
    {
    public:
        PasteButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : TemplatedButtonInternal(shortcuts)
        {}
    public:
        bool ClipboardCopied = false;
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x / 2.f, GameActionButton::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const final { return ClipboardCopied && (state == GameState::Paint || state == GameState::Empty); }
    };

    class DeleteButton : public TemplatedButton<"Delete", GameAction::Delete, false>
    {
    public:
        DeleteButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : TemplatedButtonInternal(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x, GameActionButton::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const final { return state == GameState::Paint || state == GameState::Empty; }
    };

    class EditorWidget
    {
    public:
        EditorWidget(const std::unordered_map<GameAction, std::vector<ImGuiKeyChord>>& shortcuts = {})
            : m_CopyButton(shortcuts.at(GameAction::Copy))
            , m_CutButton(shortcuts.at(GameAction::Cut))
            , m_PasteButton(shortcuts.at(GameAction::Paste))
            , m_DeleteButton(shortcuts.at(GameAction::Delete))
        {}

        SimulationControlResult Update(GameState state);
    private:
        CopyButton m_CopyButton;
        CutButton m_CutButton;
        PasteButton m_PasteButton;
        DeleteButton m_DeleteButton;
    };
}

#endif