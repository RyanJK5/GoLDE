#ifndef __NoiseWidget_h__
#define __NoiseWidget_h__

#include <cstdint>
#include <imgui/imgui.h>
#include <span>
#include <string>

#include "ActionButton.h"
#include "EditorResult.h"
#include "GameEnums.h"
#include "Graphics2D.h"
#include "SimulationControlResult.h"
#include "InputString.h"
#include "Widget.h"

namespace gol
{
    class GenerateNoiseButton : public ActionButton<EditorAction, true>
    {
    public:
        GenerateNoiseButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButton(EditorAction::GenerateNoise, shortcuts)
        {
        }
    protected:
        virtual Size2F Dimensions() const override final;
        virtual std::string Label(const EditorResult&) const override final;
        virtual bool Enabled(const EditorResult& state) const override final;
    };

    class NoiseWidget : public Widget
    {
    public:
        static constexpr int32_t BigStep = 10;
    public:
        NoiseWidget(std::span<const ImGuiKeyChord> shortcuts = {})
            : m_Button(shortcuts)
        { }

        friend Widget;
    private:
        SimulationControlResult UpdateImpl(const EditorResult& state);
    private:
        float m_Density = 0.5f;

        GenerateNoiseButton m_Button;
    };
}

#endif