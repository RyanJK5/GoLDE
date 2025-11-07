#ifndef __DelayWidget_h__
#define __DelayWidget_h__

#include <span>

#include "SimulationControlResult.h"

namespace gol
{
	class DelayWidget
	{
	public:
		DelayWidget(std::span<const ImGuiKeyChord> shortcuts = {})
		{ }
		SimulationControlResult Update(GameState state);
	private:
		int32_t m_TickDelayMs = 0;
	};
}

#endif