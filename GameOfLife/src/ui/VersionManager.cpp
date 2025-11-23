#include <optional>
#include <set>
#include <span>
#include <utility>
#include <variant>

#include "GameEnums.h"
#include "Graphics2D.h"
#include "KeyShortcut.h"
#include "SimulationControlResult.h"
#include "VersionManager.h"

gol::SimulationControlResult gol::VersionShortcutManager::Update(GameState state)
{
	if (state != GameState::Paint && state != GameState::Empty)
		return { };
	auto result = CheckShortcuts(m_UndoShortcuts, EditorAction::Undo);
	auto redoShortcuts = CheckShortcuts(m_RedoShortcuts, EditorAction::Redo);
	if (!result)
		result = redoShortcuts;
	return { .Action = result };
}

std::optional<gol::EditorAction> gol::VersionShortcutManager::CheckShortcuts(std::span<KeyShortcut> shortcuts, EditorAction targetAction)
{
	auto result = std::optional<EditorAction> {};
	for (auto&& shortcut : shortcuts)
	{
		auto active = shortcut.Active();
		if (!active)
			continue;
		
		if (!result)
			result = targetAction;
	}
	return result;
}

void gol::VersionManager::BeginPaintChange(Vec2 pos, bool insert)
{
	if (insert)
		m_UndoStack.push({ .CellsInserted = { pos } });
	else
		m_UndoStack.push({ .CellsDeleted = { pos } });
	ClearRedos();
}

void gol::VersionManager::AddPaintChange(Vec2 pos)
{
	if (m_UndoStack.empty())
		return;

	if (m_UndoStack.top().CellsInserted.size() > 0)
		m_UndoStack.top().CellsInserted.insert(pos);
	else
		m_UndoStack.top().CellsDeleted.insert(pos);
}

void gol::VersionManager::PushChange(const VersionChange& change)
{
	m_UndoStack.push(change);
	ClearRedos();
}

void gol::VersionManager::TryPushChange(std::optional<VersionChange> change)
{
	if (!change)
		return;
	m_UndoStack.push(*change);
	ClearRedos();
}

std::optional<gol::VersionChange> gol::VersionManager::Undo()
{
	if (m_UndoStack.empty())
		return std::nullopt;

	VersionChange state = std::move(m_UndoStack.top());
	m_UndoStack.pop();
	m_RedoStack.push(state);

	return state;
}

std::optional<gol::VersionChange> gol::VersionManager::Redo()
{
	if (m_RedoStack.empty())
		return std::nullopt;

	VersionChange state = std::move(m_RedoStack.top());
	m_RedoStack.pop();
	m_UndoStack.push(state);
	return state;
}

void gol::VersionManager::ClearRedos()
{
	while (!m_RedoStack.empty())
		m_RedoStack.pop();
}