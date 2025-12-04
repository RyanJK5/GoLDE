#ifndef __ActionWidget_h__
#define __ActionWidget_h__

#include <cstdint>
#include <filesystem>
#include <optional>

#include "Graphics2D.h"
#include "GameEnums.h"
	
namespace gol
{
	struct SimulationControlResult
	{
		SimulationState State = SimulationState::Empty;
		std::optional<ActionVariant> Action;
		
		std::optional<int32_t> StepCount;
		std::optional<Size2> NewDimensions;
		std::optional<int32_t> TickDelayMs;
		std::optional<std::filesystem::path> FilePath;
		int32_t NudgeSize = 0;
		bool GridLines = false;
	};
}


#endif