#ifndef __DelayWidget_h__
#define __DelayWidget_h__

#include <cstdint>
#include <imgui/imgui.h>
#include <span>

#include "GameEnums.h"
#include "SimulationControlResult.h"

namespace gol
{
	class DelayWidget
	{
	public:
		DelayWidget(std::span<const ImGuiKeyChord> = {}) { }
		SimulationControlResult Update(SimulationState state);
	private:
		int32_t m_TickDelayMs = 0;
	};
}

#endif