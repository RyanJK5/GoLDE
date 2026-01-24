#ifndef __StepWidget_h__
#define __StepWidget_h__

#include <cstdint>
#include <imgui/imgui.h>
#include <span>
#include <string>

#include "ActionButton.h"
#include "EditorResult.h"
#include "GameEnums.h"
#include "Graphics2D.h"
#include "SimulationControlResult.h"
#include "Widget.h"

namespace gol
{
    class StepButton : public ActionButton<GameAction, false>
    {
    public:
        StepButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButton(GameAction::Step, shortcuts)
        { }
    protected:
        virtual Size2F Dimensions() const override final;
        virtual std::string Label(const EditorResult&) const override final;
        virtual bool Enabled(const EditorResult& state) const override final;
    };

	class StepWidget : public Widget
	{
    public:
        static constexpr int32_t BigStep = 10;
        static constexpr int32_t StepWarning = 100;
	public:
        StepWidget(std::span<const ImGuiKeyChord> shortcuts = {})
            : m_Button(shortcuts)
        { }
    friend Widget;

    private:
		SimulationControlResult UpdateImpl(const EditorResult& state);
	private:
        int32_t m_StepCount = 1;

        StepButton m_Button;
	};
}

#endif