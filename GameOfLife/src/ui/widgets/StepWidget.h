#ifndef __StepWidget_h__
#define __StepWidget_h__

#include <cstdint>
#include <font-awesome/IconsFontAwesome7.h>
#include <imgui/imgui.h>
#include <span>
#include <string>

#include "ActionButton.h"
#include "GameEnums.h"
#include "Graphics2D.h"
#include "SimulationControlResult.h"

namespace gol
{
    class StepButton : public ActionButton<GameAction, false>
    {
    public:
        StepButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButton(GameAction::Step, shortcuts)
        { }
    protected:
        virtual Size2F Dimensions() const override final { return { ImGui::GetContentRegionAvail().x, ActionButton::DefaultButtonHeight }; }
        virtual std::string Label(EditorState) const override final { return ICON_FA_FORWARD_STEP; }
        virtual bool Enabled(EditorState state) const override final { return state.State == SimulationState::Paint || state.State == SimulationState::Paused; }
    };

	class StepWidget
	{
    public:
        static constexpr int32_t BigStep = 10;
        static constexpr int32_t StepWarning = 100;
	public:
        StepWidget(std::span<const ImGuiKeyChord> shortcuts = {})
            : m_Button(shortcuts)
        { }

		SimulationControlResult Update(EditorState state);
	private:
        int32_t m_StepCount = 1;

        StepButton m_Button;
	};
}

#endif