#ifndef __StepWidget_h__
#define __StepWidget_h__

#include <cstdint>
#include <imgui/imgui.h>
#include <span>

#include "GameActionButton.h"
#include "GameEnums.h"
#include "Graphics2D.h"
#include "SimulationControlResult.h"

namespace gol
{
    class StepButton : public TemplatedButton<"Step", GameAction::Step, false>
    {
    public:
        StepButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : TemplatedButtonInternal(shortcuts)
        { }
    protected:
        virtual Size2F Dimensions() const override final { return { ImGui::GetContentRegionAvail().x, GameActionButton::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const override final { return state == GameState::Paint || state == GameState::Paused; }
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

		SimulationControlResult Update(GameState state);
	private:
        int32_t m_StepCount = 1;

        StepButton m_Button;
	};
}

#endif