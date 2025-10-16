#ifndef __GameActionButton_h__
#define __GameActionButton_h__

#include <string_view>
#include <vector>
#include <functional>

#include "Graphics2D.h"
#include "GameEnums.h"
#include "KeyShortcut.h"

namespace gol
{
	struct DrawInfo
	{
		uint32_t SimulationTextureID;
		GameState State;
		bool GridDead;
	};

	class GameActionButton
	{
	public:
		GameActionButton(
			std::string_view label, 
			GameAction actionReturn,
			Size2F size,
			const std::function<bool(const DrawInfo&)>& enabledCheck,
			std::initializer_list<KeyShortcut> shortcuts,
			bool lineBreak = false
		);

		GameAction Update(const DrawInfo& state);
	private:
		std::string_view m_Label;
		GameAction m_Return;
		Size2F m_Size;

		std::function<bool(const DrawInfo&)> m_Enabled;

		std::vector<KeyShortcut> m_Shortcuts;
		bool m_LineBreak;
	};
}

#endif