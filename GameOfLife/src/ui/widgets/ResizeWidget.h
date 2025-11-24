#ifndef __ResizeWidget_h__
#define __ResizeWidget_h__

#include <imgui/imgui.h>
#include <span>

#include "ActionButton.h"
#include "GameEnums.h"
#include "Graphics2D.h"
#include "SimulationControlResult.h"

namespace gol
{
    class ResizeButton : public ActionButton<EditorAction, false>
    {
    public:
        ResizeButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButton(EditorAction::Resize, shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x, ActionButton::DefaultButtonHeight }; }
        virtual std::string Label(EditorState) const override final { return "Apply"; }
        virtual bool Enabled(EditorState state) const final { return state.State == SimulationState::Paint || state.State == SimulationState::Empty; }
    };

    class ResizeWidget
    {
    public:
        ResizeWidget(std::span<const ImGuiKeyChord> shortcuts = {})
            : m_Button(shortcuts)
        { }

        SimulationControlResult Update(EditorState state);
    private:
        ResizeButton m_Button;
        Size2 m_Dimensions;
    };
}

#endif