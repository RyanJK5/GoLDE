#ifndef __GameEnums_h__
#define __GameEnums_h__

#include <concepts>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

namespace gol
{
	enum class SimulationState
	{
		Paint, Simulation, Paused, Empty
	};

	enum class GameAction
	{
		None,
		Start,
		Pause,
		Resume,
		Restart,
		Reset,
		Clear,
		Step
	};

	enum class EditorAction
	{
		Resize,
		Undo,
		Redo
	};

	enum class SelectionAction 
	{
		Rotate,
		Select,
		Deselect,
		Delete,
		Copy,
		Cut,
		Paste,

		NudgeLeft,
		NudgeRight,
		NudgeUp,
		NudgeDown
	};

	using ActionVariant = std::variant<GameAction, EditorAction, SelectionAction>;

	namespace Actions
	{
		inline const std::unordered_map<std::string_view, GameAction> GameActionDefinitions = {
			{ "start",       GameAction::Start      },
			{ "pause",       GameAction::Pause      },
			{ "resume",      GameAction::Resume     },
			{ "restart",     GameAction::Restart    },
			{ "reset",       GameAction::Reset      },
			{ "clear",       GameAction::Clear      },
			{ "step",        GameAction::Step       },
		};
		inline const std::unordered_map<std::string_view, EditorAction> EditorActionDefinitions = {
			{ "resize",      EditorAction::Resize     },
			{ "undo",        EditorAction::Undo       },
			{ "redo",        EditorAction::Redo       }
		};
		inline const std::unordered_map<std::string_view, SelectionAction> SelectionActionDefinitions = {
			{ "rotate",      SelectionAction::Rotate     },
			{ "deselect",    SelectionAction::Deselect   },
			{ "delete",      SelectionAction::Delete     },
			{ "copy",        SelectionAction::Copy       },
			{ "cut",         SelectionAction::Cut        },
			{ "paste",       SelectionAction::Paste      },

			{ "nudge_left",  SelectionAction::NudgeLeft  },
			{ "nudge_right", SelectionAction::NudgeRight },
			{ "nudge_up",    SelectionAction::NudgeUp    },
			{ "nudge_down",  SelectionAction::NudgeDown  },
		};

		std::string ToString(ActionVariant action);
	};


	template <typename T>
	concept ActionType = 
		std::same_as<T, GameAction>   || 
		std::same_as<T, EditorAction> || 
		std::same_as<T, SelectionAction>;

	enum class EditorMode
	{
		None, 
		Insert, 
		Delete, 
		Select
	};
}

#endif