#ifndef __EditorWidget_h__
#define __EditorWidget_h__

#include <imgui/imgui.h>
#include <span>
#include <unordered_map>
#include <vector>

#include "ActionButton.h"
#include "GameEnums.h"
#include "EditorResult.h"
#include "Graphics2D.h"
#include "SimulationControlResult.h"
#include "Widget.h"

namespace gol
{
    class CopyButton : public ActionButton<SelectionAction, true>
    {
    public:
        CopyButton(std::span<const ImGuiKeyChord> shortcuts = {});
    protected:
        virtual Size2F Dimensions() const final;
        virtual std::string Label(const EditorResult&) const override final;
        virtual bool Enabled(const EditorResult& state) const final;
    };

    class CutButton : public ActionButton<SelectionAction, false>
    {
    public:
        CutButton(std::span<const ImGuiKeyChord> shortcuts = {});
    protected:
        virtual Size2F Dimensions() const final;
        virtual std::string Label(const EditorResult&) const override final;
        virtual bool Enabled(const EditorResult& state) const final;
    };

    class PasteButton : public ActionButton<SelectionAction, false>
    {
    public:
        PasteButton(std::span<const ImGuiKeyChord> shortcuts = {});
    protected:
        virtual Size2F Dimensions() const final;
        virtual std::string Label(const EditorResult&) const override final;
        virtual bool Enabled(const EditorResult& state) const final;
    };

    class DeleteButton : public ActionButton<SelectionAction, false>
    {
    public:
        DeleteButton(std::span<const ImGuiKeyChord> shortcuts = {});
    protected:
        virtual Size2F Dimensions() const final;
        virtual std::string Label(const EditorResult&) const override final;
        virtual bool Enabled(const EditorResult& state) const final;
    };

    class DeselectButton : public ActionButton<SelectionAction, true>
    {
    public:
        DeselectButton(std::span<const ImGuiKeyChord> shortcuts = {});
    protected:
        virtual Size2F Dimensions() const final;
        virtual std::string Label(const EditorResult&) const override final;
        virtual bool Enabled(const EditorResult& state) const final;
    };

    class RotateButton : public ActionButton<SelectionAction, false>
    {
    public:
        RotateButton(std::span<const ImGuiKeyChord> shortcuts = {});
    protected:
        virtual Size2F Dimensions() const final;
        virtual std::string Label(const EditorResult&) const override final;
        virtual bool Enabled(const EditorResult& state) const final;
    };

    class FlipHorizontalButton : public ActionButton<SelectionAction, false>
    {
    public:
        FlipHorizontalButton(std::span<const ImGuiKeyChord> shortcuts = {});
    protected:
        virtual Size2F Dimensions() const final;
        virtual std::string Label(const EditorResult&) const override final;
        virtual bool Enabled(const EditorResult& state) const final;
    };

    class FlipVerticalButton : public ActionButton<SelectionAction, false>
    {
    public:
        FlipVerticalButton(std::span<const ImGuiKeyChord> shortcuts = {});
    protected:
        virtual Size2F Dimensions() const final;
        virtual std::string Label(const EditorResult&) const override final;
        virtual bool Enabled(const EditorResult& state) const final;
    };

    class UndoButton : public ActionButton<EditorAction, true>
    {
    public:
        UndoButton(std::span<const ImGuiKeyChord> shortcuts = {});
    protected:
        virtual Size2F Dimensions() const final;
        virtual std::string Label(const EditorResult&) const override final;
        virtual bool Enabled(const EditorResult& state) const final;
    };

    class RedoButton : public ActionButton<EditorAction, false>
    {
    public:
        RedoButton(std::span<const ImGuiKeyChord> shortcuts = {});
    protected:
        virtual Size2F Dimensions() const final;
        virtual std::string Label(const EditorResult&) const override final;
        virtual bool Enabled(const EditorResult& state) const final;
    };

    class EditorWidget : public Widget
    {
    public:
        EditorWidget(const std::unordered_map<ActionVariant, std::vector<ImGuiKeyChord>>& shortcuts = {});
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
        FlipHorizontalButton m_FlipHorizontalButton;
        FlipVerticalButton m_FlipVerticalButton;
        UndoButton m_UndoButton;
        RedoButton m_RedoButton;
    };
}

#endif