#ifndef __VersionManager_h__
#define __VersionManager_h__

#include <optional>
#include <stack>
#include <unordered_set>
#include <utility>

#include "GameEnums.h"
#include "GameGrid.h"
#include "Graphics2D.h"

namespace gol {
struct VersionChange {
  std::optional<ActionVariant> Action;
  std::optional<std::pair<GameGrid, Size2>> GridResize;
  std::optional<Rect> SelectionBounds;
  LifeHashSet CellsInserted;
  LifeHashSet CellsDeleted;
  Vec2 NudgeTranslation;

  bool InsertedIntoSelection() const { return SelectionBounds.has_value(); }
};

class VersionManager {
public:
  void BeginPaintChange(Vec2 pos, bool insert);
  void AddPaintChange(Vec2 pos);

  void PushChange(const VersionChange &change);
  void TryPushChange(std::optional<VersionChange> change);

  std::optional<VersionChange> Undo();
  std::optional<VersionChange> Redo();

  void Save() { m_LastSavedHeight = m_EditHeight; }
  bool IsSaved() const { return m_EditHeight == m_LastSavedHeight; }

  bool UndosAvailable() const { return !m_UndoStack.empty(); }
  bool RedosAvailable() const { return !m_RedoStack.empty(); }

private:
  bool BreakingChange(const VersionChange &change) const;

  void ClearRedos();

private:
  size_t m_EditHeight = 0;
  size_t m_LastSavedHeight = 0;

  std::stack<VersionChange> m_UndoStack;
  std::stack<VersionChange> m_RedoStack;
};
} // namespace gol

#endif