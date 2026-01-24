#ifndef __Widget_h__
#define __Widget_h__

#include <concepts>

#include "ActionButton.h"
#include "EditorResult.h"
#include "GameEnums.h"
#include "SimulationControlResult.h"

namespace gol
{
	class Widget;

	class Widget
	{
	public:
		SimulationControlResult Update(this auto&& self, const EditorResult& state) { return self.UpdateImpl(state); }
	protected:
		template <ActionType ActType>
		inline static void UpdateResult(SimulationControlResult& result, const ActionButtonResult<ActType>& update)
		{
			if (!result.Action)
				result.Action = update.Action;
			if (!result.FromShortcut)
				result.FromShortcut = update.FromShortcut;
		}
	};
}

#endif