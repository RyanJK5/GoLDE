#ifndef __ResizeWidget_h__
#define __ResizeWidget_h__

#include <imgui/imgui.h>
#include <span>

#include "GameActionButton.h"
#include "GameEnums.h"
#include "Graphics2D.h"
#include "SimulationControlResult.h"

namespace gol
{
    class ResizeButton : public TemplatedButton<"Apply", GameAction::Resize, false>
    {
    public:
        ResizeButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : TemplatedButtonInternal(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x, GameActionButton::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const final { return state == GameState::Paint || state == GameState::Empty; }
    };

    class ResizeWidget
    {
    public:
        ResizeWidget(std::span<const ImGuiKeyChord> shortcuts = {})
            : m_Button(shortcuts)
        { }

        SimulationControlResult Update(GameState state);
    private:
        ResizeButton m_Button;
        Size2 m_Dimensions;
    };
}

#endif