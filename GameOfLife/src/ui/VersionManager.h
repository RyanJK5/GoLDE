	#ifndef __VersionManager_h__
#define __VersionManager_h__

#include <imgui/imgui.h>
#include <optional>
#include <ranges>
#include <set>
#include <span>
#include <stack>
#include <utility>
#include <vector>

#include "GameEnums.h"
#include "GameGrid.h"
#include "Graphics2D.h"
#include "KeyShortcut.h"
#include "SimulationControlResult.h"

namespace gol
{
	struct VersionChange
	{
		std::optional<ActionVariant> Action;
		std::optional<std::pair<GameGrid, Size2>> GridResize;
		std::optional<Rect> SelectionBounds;
		std::set<Vec2> CellsInserted;
		std::set<Vec2> CellsDeleted;
		Vec2 NudgeTranslation;

		bool InsertedIntoSelection() const { return SelectionBounds.has_value(); }
	};

	class VersionManager
	{
	public:
		void BeginPaintChange(Vec2 pos, bool insert);
		void AddPaintChange(Vec2 pos);

		void PushChange(const VersionChange& change);
		void TryPushChange(std::optional<VersionChange> change);

		std::optional<VersionChange> Undo();
		std::optional<VersionChange> Redo();

		bool UndosAvailable() const { return !m_UndoStack.empty(); }
		bool RedosAvailable() const { return !m_RedoStack.empty(); }
	private:
		void ClearRedos();
	private:
		std::stack<VersionChange> m_UndoStack;
		std::stack<VersionChange> m_RedoStack;
	};
}

#endif