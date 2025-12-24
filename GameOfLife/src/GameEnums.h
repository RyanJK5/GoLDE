#ifndef __GameEnums_h__
#define __GameEnums_h__

#include <concepts>
#include <filesystem>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

namespace gol
{
	enum class SimulationState
	{
		None, Paint, Simulation, Paused, Empty
	};

	struct EditorResult
	{
		std::filesystem::path CurrentFilePath;
		SimulationState State = SimulationState::Paint;
		bool Active = true;
		bool Closing = false;
		bool SelectionActive = false;
		bool UndosAvailable = false;
		bool RedosAvailable = false;
		bool HasUnsavedChanges = false;
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
		Redo,
		Save,
		NewFile,
		UpdateFile,
		Load,
		Close
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
		NudgeDown,

		FlipVertically,
		FlipHorizontally
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
			{ "redo",        EditorAction::Redo       },
			{ "save",        EditorAction::Save       },
			{ "new",         EditorAction::NewFile    },
			{ "update",      EditorAction::UpdateFile },
			{ "load",        EditorAction::Load       },
			{ "close",       EditorAction::Close      },
		};
		inline const std::unordered_map<std::string_view, SelectionAction> SelectionActionDefinitions = {
			{ "rotate",           SelectionAction::Rotate            },
			{ "deselect",         SelectionAction::Deselect          },
			{ "delete",           SelectionAction::Delete            },
			{ "copy",             SelectionAction::Copy              },
			{ "cut",              SelectionAction::Cut               },
			{ "paste",            SelectionAction::Paste             },
			{ "nudge_left",       SelectionAction::NudgeLeft         },
			{ "nudge_right",      SelectionAction::NudgeRight        },
			{ "nudge_up",         SelectionAction::NudgeUp           },
			{ "nudge_down",       SelectionAction::NudgeDown         },
			{ "flip_horizontal",  SelectionAction::FlipHorizontally  },
			{ "flip_vertical",    SelectionAction::FlipVertically    }
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