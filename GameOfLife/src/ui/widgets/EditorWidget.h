#ifndef __EditorWidget_h__
#define __EditorWidget_h__

#include <imgui/imgui.h>
#include <font-awesome/IconsFontAwesome7.h>
#include <span>
#include <unordered_map>
#include <vector>

#include "ActionButton.h"
#include "GameEnums.h"
#include "Graphics2D.h"
#include "SimulationControlResult.h"


namespace gol
{
    class CopyButton : public ActionButton<SelectionAction, true>
    {
    public:
        CopyButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButton(SelectionAction::Copy, shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x / 4.f, ActionButton::DefaultButtonHeight }; }
        virtual std::string Label(SimulationState) const override final { return ICON_FA_COPY; }
        virtual bool Enabled(SimulationState state) const final { return state == SimulationState::Paint || state == SimulationState::Empty || state == SimulationState::Paused; }
    };

    class CutButton : public ActionButton<SelectionAction, false>
    {
    public:
        CutButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButton(SelectionAction::Cut, shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x / 3.f, ActionButton::DefaultButtonHeight }; }
        virtual std::string Label(SimulationState) const override final { return ICON_FA_SCISSORS; }
        virtual bool Enabled(SimulationState state) const final { return state == SimulationState::Paint || state == SimulationState::Empty; }
    };

    class PasteButton : public ActionButton<SelectionAction, false>
    {
    public:
        PasteButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButton(SelectionAction::Paste, shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x / 2.f, ActionButton::DefaultButtonHeight }; }
        virtual std::string Label(SimulationState) const override final { return ICON_FA_PASTE; }
        virtual bool Enabled(SimulationState state) const final { return ImGui::GetClipboardText() && (state == SimulationState::Paint || state == SimulationState::Empty); }
    };

    class DeleteButton : public ActionButton<SelectionAction, false>
    {
    public:
        DeleteButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButton(SelectionAction::Delete, shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x, ActionButton::DefaultButtonHeight }; }
        virtual std::string Label(SimulationState) const override final { return ICON_FA_XMARK; }
        virtual bool Enabled(SimulationState state) const final { return state == SimulationState::Paint || state == SimulationState::Empty; }
    };

    class EditorWidget
    {
    public:
        EditorWidget(const std::unordered_map<ActionVariant, std::vector<ImGuiKeyChord>>& shortcuts = {})
            : m_CopyButton  (shortcuts.at(SelectionAction::Copy  ))
            , m_CutButton   (shortcuts.at(SelectionAction::Cut   ))
            , m_PasteButton (shortcuts.at(SelectionAction::Paste ))
            , m_DeleteButton(shortcuts.at(SelectionAction::Delete))
        {}

        SimulationControlResult Update(SimulationState state);
    private:
        CopyButton m_CopyButton;
        CutButton m_CutButton;
        PasteButton m_PasteButton;
        DeleteButton m_DeleteButton;
    };
}

#endif