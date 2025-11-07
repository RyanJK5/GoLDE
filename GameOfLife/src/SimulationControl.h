#ifndef __SimulationControl_h__
#define __SimulationControl_h__

#include <memory>
#include <vector>
#include <filesystem>

#include "GameEnums.h"
#include "StepWidget.h"
#include "ResizeWidget.h"
#include "ExecutionWidget.h"

namespace gol
{
	class SimulationControl
	{
	public:
		SimulationControl(const std::filesystem::path& shortcuts);

		SimulationControlResult Update(GameState state);
	private:
		void CreateButtons(const std::unordered_map<GameAction, std::vector<ImGuiKeyChord>>& shortcuts);

		void FillResuts(SimulationControlResult& current, const SimulationControlResult& update) const;
	private:
		static constexpr int32_t BigStep = 100;
		static constexpr int32_t StepWarning = 100;
	private:
		ExecutionWidget m_ExecutionWidget;
		ResizeWidget m_ResizeWidget;
		StepWidget m_StepWidget;
	};
}

#endif