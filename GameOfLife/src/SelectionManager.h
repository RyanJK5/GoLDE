#ifndef __SelectionManager_h__
#define __SelectionManager_h__

#include "GameGrid.h"
#include "VersionManager.h"
#include <optional>
#include "Graphics2D.h"
#include <cstdint>
#include <set>
#include "GameEnums.h"

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

		std::optional<VersionChange> Deselect(GameGrid& grid);

		std::optional<VersionChange> Copy(GameGrid& grid);

		std::optional<VersionChange> Paste(GameGrid& grid, std::optional<Vec2> gridPos);

		std::optional<VersionChange> Delete();
		
		std::optional<VersionChange> Cut();

		std::optional<VersionChange> Rotate(bool clockwise);

		std::optional<VersionChange> Nudge(Vec2 translation);

		void HandleVersionChange(GameAction undoRedo, GameGrid& grid, const VersionChange& change);

		Rect SelectionBounds() const;

		bool GridAlive() const;
		const std::set<Vec2>& GridData() const;
		int32_t SelectedPopulation() const;

		bool CanDrawSelection() const;
		bool CanDrawLargeSelection() const;
		bool CanDrawGrid() const;
	private:
		void RestoreGridVersion(GameAction undoRedo, GameGrid& grid, const VersionChange& change);

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
