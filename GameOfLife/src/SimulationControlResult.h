#ifndef __ActionWidget_h__
#define __ActionWidget_h__

#include <optional>

#include "Graphics2D.h"
#include "GameEnums.h"

namespace gol
{
	struct SimulationControlResult
	{
		GameState State;
		GameAction Action = GameAction::None;
		std::optional<int32_t> StepCount;
		std::optional<Size2> NewDimensions;
		std::optional<int32_t> TickDelayMs;
	};
}


#endif