#include <optional>
#include <utility>

#include "GameEnums.h"
#include "Graphics2D.h"
#include "VersionManager.h"

void gol::VersionManager::BeginPaintChange(Vec2 pos, bool insert) {
  if (insert)
    PushChange({.CellsInserted = {pos}});
  else
    PushChange({.CellsDeleted = {pos}});
}

void gol::VersionManager::AddPaintChange(Vec2 pos) {
  if (m_UndoStack.empty())
    return;

  if (m_UndoStack.top().CellsInserted.size() > 0)
    m_UndoStack.top().CellsInserted.insert(pos);
  else
    m_UndoStack.top().CellsDeleted.insert(pos);
}

void gol::VersionManager::PushChange(const VersionChange &change) {
  if (BreakingChange(change))
    m_EditHeight++;
  m_UndoStack.push(change);
  ClearRedos();
}

void gol::VersionManager::TryPushChange(std::optional<VersionChange> change) {
  if (!change)
    return;
  PushChange(*change);
}

std::optional<gol::VersionChange> gol::VersionManager::Undo() {
  if (m_UndoStack.empty())
    return std::nullopt;

  VersionChange state = std::move(m_UndoStack.top());
  if (BreakingChange(state))
    m_EditHeight--;
  m_UndoStack.pop();
  m_RedoStack.push(state);

  return state;
}

std::optional<gol::VersionChange> gol::VersionManager::Redo() {
  if (m_RedoStack.empty())
    return std::nullopt;

  VersionChange state = std::move(m_RedoStack.top());
  if (BreakingChange(state))
    m_EditHeight++;

  m_RedoStack.pop();
  m_UndoStack.push(state);
  return state;
}

bool gol::VersionManager::BreakingChange(const VersionChange &change) const {
  return !change.Action ||
         (change.Action != ActionVariant{SelectionAction::Select} &&
          change.Action != ActionVariant{SelectionAction::Deselect} &&
          change.Action != ActionVariant{SelectionAction::Copy});
}

void gol::VersionManager::ClearRedos() {
  while (!m_RedoStack.empty())
    m_RedoStack.pop();
}