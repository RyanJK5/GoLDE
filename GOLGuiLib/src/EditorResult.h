#ifndef __EditorResult_h__
#define __EditorResult_h__

#include <filesystem>

#include "GameEnums.h"

namespace gol
{
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
}

#endif