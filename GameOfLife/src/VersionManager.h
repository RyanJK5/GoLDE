#ifndef __VersionManager_h__
#define __VersionManager_h__

#include <set>
#include <vector>
#include <stack>
#include <ranges>
#include <optional>
#include <span>

#include "SimulationControlResult.h"
#include "KeyShortcut.h"
#include "GameEnums.h"
#include "Graphics2D.h"
#include "imgui.h"

namespace gol
{
	struct VersionChange
	{
		GameAction Action = GameAction::None;
		std::optional<Rect> SelectionBounds;
		std::set<Vec2> CellsInserted;
		std::set<Vec2> CellsDeleted;
		Vec2 NudgeTranslation;

		bool InsertedIntoSelection() const { return SelectionBounds.has_value(); }
	};

	class VersionShortcutManager
	{
	public:
		SimulationControlResult Update(GameState state);
		
		VersionShortcutManager(std::span<const ImGuiKeyChord> undoShortcuts, std::span<const ImGuiKeyChord> redoShortcuts)
			: m_UndoShortcuts(undoShortcuts | KeyShortcut::MapChordsToVector)
			, m_RedoShortcuts(redoShortcuts | KeyShortcut::MapChordsToVector)
		{ }
	private:
		GameAction CheckShortcuts(std::span<KeyShortcut> shortcuts, GameAction targetAction);
	private:
		std::vector<KeyShortcut> m_UndoShortcuts;
		std::vector<KeyShortcut> m_RedoShortcuts;
	};

	class VersionManager
	{
	public:
		void BeginPaintChange(Vec2 pos, bool insert);
		void AddPaintChange(Vec2 pos);
		void AddBatchChange(const std::set<Vec2>& positions, GameAction action, bool insert);

		void TryPushChange(std::optional<VersionChange> change);
		void AddSelectionChange(const VersionChange& change);
		const VersionChange* PastChange() const;
		
		void AddActionsChange(GameAction action);

		std::optional<VersionChange> Undo();
		std::optional<VersionChange> Redo();
	private:
		void ClearRedos();
	private:
		std::stack<VersionChange> m_UndoStack;
		std::stack<VersionChange> m_RedoStack;
	};
}

#endif