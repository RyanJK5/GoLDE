#ifndef __SimulationControl_h__
#define __SimulationControl_h__

#include <memory>
#include <vector>
#include <filesystem>

#include "GameEnums.h"
#include "SimulationControlButtons.h"

namespace gol
{
	struct SimulationControlResult
	{
		GameState State;
		GameAction Action = GameAction::None;
		int32_t StepCount = 0;
		Size2 NewDimensions = Size2 { 0, 0 };
	};

	class SimulationControl
	{
	public:
		SimulationControl(const std::filesystem::path& shortcuts);

		SimulationControlResult Update(GameState state);
	private:
		GameAction UpdateStepButton(GameState state);
		GameAction UpdateDimensions(GameState state);

		void CreateButtons(const std::unordered_map<GameAction, std::vector<ImGuiKeyChord>>& shortcuts);
	private:
		static constexpr int32_t BigStep = 100;
		static constexpr int32_t StepWarning = 100;
	private:
		std::vector<std::unique_ptr<GameActionButton>> m_BasicButtons;
		
		StepButton m_StepButton;
		int32_t m_StepCount = 1;

		ResizeButton m_ResizeButton;
		Size2 m_Dimensions;
	};
}

#endif