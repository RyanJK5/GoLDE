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
#include "Widget.h"

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
        virtual std::string Label(const EditorResult&) const override final { return ICON_FA_COPY; }
        virtual bool Enabled(const EditorResult& state) const final { return state.SelectionActive; }
    };

    class CutButton : public ActionButton<SelectionAction, false>
    {
    public:
        CutButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButton(SelectionAction::Cut, shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x / 2.f, ActionButton::DefaultButtonHeight }; }
        virtual std::string Label(const EditorResult&) const override final { return ICON_FA_SCISSORS; }
        virtual bool Enabled(const EditorResult& state) const final { return state.SelectionActive && (state.State == SimulationState::Paint || state.State == SimulationState::Empty); }
    };

    class PasteButton : public ActionButton<SelectionAction, false>
    {
    public:
        PasteButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButton(SelectionAction::Paste, shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x / 3.f, ActionButton::DefaultButtonHeight }; }
        virtual std::string Label(const EditorResult&) const override final { return ICON_FA_PASTE; }
        virtual bool Enabled(const EditorResult& state) const final { return ImGui::GetClipboardText() && (state.State == SimulationState::Paint || state.State == SimulationState::Empty); }
    };

    class DeleteButton : public ActionButton<SelectionAction, false>
    {
    public:
        DeleteButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButton(SelectionAction::Delete, shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x, ActionButton::DefaultButtonHeight }; }
        virtual std::string Label(const EditorResult&) const override final { return ICON_FA_DELETE_LEFT; }
        virtual bool Enabled(const EditorResult& state) const final { return state.SelectionActive && (state.State == SimulationState::Paint || state.State == SimulationState::Empty); }
    };

    class DeselectButton : public ActionButton<SelectionAction, true>
    {
    public:
        DeselectButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButton(SelectionAction::Deselect, shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x / 4.f, ActionButton::DefaultButtonHeight }; }
        virtual std::string Label(const EditorResult&) const override final { return ICON_FA_OBJECT_UNGROUP; }
        virtual bool Enabled(const EditorResult& state) const final { return state.SelectionActive; }
    };

    class RotateButton : public ActionButton<SelectionAction, false>
    {
    public:
        RotateButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButton(SelectionAction::Rotate, shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x / 3.f, ActionButton::DefaultButtonHeight }; }
        virtual std::string Label(const EditorResult&) const override final { return ICON_FA_ROTATE; }
        virtual bool Enabled(const EditorResult& state) const final { return state.SelectionActive && (state.State == SimulationState::Paint || state.State == SimulationState::Empty); }
    };

    class UndoButton : public ActionButton<EditorAction, false>
    {
    public:
        UndoButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButton(EditorAction::Undo, shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x / 2.f, ActionButton::DefaultButtonHeight }; }
        virtual std::string Label(const EditorResult&) const override final { return ICON_FA_ARROW_ROTATE_LEFT; }
        virtual bool Enabled(const EditorResult& state) const final { return state.UndosAvailable && (state.State == SimulationState::Paint || state.State == SimulationState::Empty); }
    };

    class RedoButton : public ActionButton<EditorAction, false>
    {
    public:
        RedoButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButton(EditorAction::Redo, shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x, ActionButton::DefaultButtonHeight }; }
        virtual std::string Label(const EditorResult&) const override final { return ICON_FA_ARROW_ROTATE_RIGHT; }
        virtual bool Enabled(const EditorResult& state) const final { return state.RedosAvailable && (state.State == SimulationState::Paint || state.State == SimulationState::Empty); }
    };

    class EditorWidget : public Widget
    {
    public:
        EditorWidget(const std::unordered_map<ActionVariant, std::vector<ImGuiKeyChord>>& shortcuts = {})
            : m_CopyButton    (shortcuts.at(SelectionAction::Copy    ))
            , m_CutButton     (shortcuts.at(SelectionAction::Cut     ))
            , m_PasteButton   (shortcuts.at(SelectionAction::Paste   ))
            , m_DeleteButton  (shortcuts.at(SelectionAction::Delete  ))
			, m_DeselectButton(shortcuts.at(SelectionAction::Deselect))
			, m_RotateButton  (shortcuts.at(SelectionAction::Rotate  ))
			, m_UndoButton    (shortcuts.at(EditorAction   ::Undo    ))
			, m_RedoButton    (shortcuts.at(EditorAction   ::Redo    ))
        {}
        friend Widget;
    public:
        SimulationControlResult UpdateImpl(const EditorResult& state);
    private:
        CopyButton m_CopyButton;
        CutButton m_CutButton;
        PasteButton m_PasteButton;
        DeleteButton m_DeleteButton;

		DeselectButton m_DeselectButton;
		RotateButton m_RotateButton;
		UndoButton m_UndoButton;
		RedoButton m_RedoButton;
    };
}

#endif