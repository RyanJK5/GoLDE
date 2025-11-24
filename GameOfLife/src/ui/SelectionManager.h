#ifndef __SelectionManager_h__
#define __SelectionManager_h__

#include <cstdint>
#include <expected>
#include <optional>
#include <set>

#include "GameEnums.h"
#include "GameGrid.h"
#include "Graphics2D.h"
#include "VersionManager.h"

namespace gol
{
	struct SelectionUpdateResult
	{
		std::optional<VersionChange> Change;
		bool BeginSelection = false;
	};

	class SelectionManager
	{
	public:
		SelectionUpdateResult UpdateSelectionArea(GameGrid& grid, Vec2 gridPos);

		bool TryResetSelection();

		std::optional<VersionChange> Deselect(GameGrid& grid);

		std::optional<VersionChange> Copy(GameGrid& grid);

		std::expected<VersionChange, int32_t> Paste(std::optional<Vec2> gridPos, uint32_t warnThreshold);

		std::optional<VersionChange> Delete();
		
		std::optional<VersionChange> Cut();

		std::optional<VersionChange> Rotate(bool clockwise);

		std::optional<VersionChange> Nudge(Vec2 translation);

		std::optional<VersionChange> HandleAction(SelectionAction action, GameGrid& grid, int32_t nudgeSize);
		void HandleVersionChange(EditorAction undoRedo, GameGrid& grid, const VersionChange& change);

		Rect SelectionBounds() const;

		bool GridAlive() const;
		const std::set<Vec2>& GridData() const;
		int64_t SelectedPopulation() const;

		bool CanDrawSelection() const;
		bool CanDrawLargeSelection() const;
		bool CanDrawGrid() const;
	private:
		void RestoreGridVersion(EditorAction undoRedo, GameGrid& grid, const VersionChange& change);

		void SetSelectionBounds(const Rect& bounds);

		std::optional<VersionChange> Delete(bool cut);

		Vec2 RotatePoint(Vec2F center, Vec2F point, bool clockwise);
	private:
		bool m_RotationParity = false;

		std::optional<Vec2> m_AnchorSelection;
		std::optional<Vec2> m_SentinelSelection;
		std::optional<GameGrid> m_Selected;
	};
}

#endif
