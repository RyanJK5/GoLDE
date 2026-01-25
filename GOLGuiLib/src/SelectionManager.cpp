#include <algorithm>
#include <cmath>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <imgui/imgui.h>
#include <optional>
#include <set>
#include <utility>
#include <variant>

#include "GameEnums.h"
#include "GameGrid.h"
#include "Graphics2D.h"
#include "RLEEncoder.h"
#include "SelectionManager.h"
#include "VersionManager.h"

gol::SelectionUpdateResult gol::SelectionManager::UpdateSelectionArea(GameGrid& grid, Vec2 gridPos)
{
    const bool shiftDown = ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift);
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && shiftDown)
    {
        m_SentinelSelection = gridPos;
        if (!m_AnchorSelection)
            m_AnchorSelection = gridPos;
        return { .BeginSelection = true };
    }

    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) && (m_AnchorSelection != m_SentinelSelection))
    {
        if (m_Selected && m_LockSelection)
            return { .BeginSelection = false };
        if (m_Selected && !m_LockSelection)
            return UpdateUnlockedSelection(gridPos);

        m_Selected = grid.SubRegion(SelectionBounds());
        auto change = VersionChange
        {
            .Action = SelectionAction::Select,
            .SelectionBounds = SelectionBounds(),
            .CellsInserted = m_Selected->Data(),
            .CellsDeleted = grid.ReadRegion(SelectionBounds())
        };
        grid.ClearRegion(SelectionBounds());

        return { .Change = change, .BeginSelection = false };
    }

    auto deselectChange = Deselect(grid);
    m_AnchorSelection = gridPos;
    m_SentinelSelection = gridPos;
    return { .Change = deselectChange, .BeginSelection = false };
}

gol::SelectionUpdateResult gol::SelectionManager::UpdateUnlockedSelection(gol::Vec2& gridPos)
{
    if (gridPos == *m_SentinelSelection && m_UnlockedOriginalPosition)
    {
        auto result = SelectionUpdateResult
        {
            .Change = VersionChange
            {
                .Action = SelectionAction::NudgeDown,
                .SelectionBounds = SelectionBounds(),
                .NudgeTranslation = gridPos - *m_UnlockedOriginalPosition
            },
            .BeginSelection = false
        };
        m_UnlockedOriginalPosition = std::nullopt;
        return result;
    }
    else if ((std::abs(ImGui::GetIO().MouseDelta.x) <= 1 && std::abs(ImGui::GetIO().MouseDelta.y) <= 1))
        return { .BeginSelection = false };

    if (!m_UnlockedOriginalPosition)
        m_UnlockedOriginalPosition = m_SentinelSelection;
    auto sentinelOffset = *m_SentinelSelection - *m_AnchorSelection;
    m_SentinelSelection = gridPos;
    m_AnchorSelection = gridPos - sentinelOffset;

    return { .BeginSelection = false };
}

bool gol::SelectionManager::TryResetSelection()
{
    if (CanDrawGrid())
        return false;
    m_AnchorSelection = std::nullopt;
    m_SentinelSelection = std::nullopt;
    m_Selected = std::nullopt;
    return true;
}

std::optional<gol::VersionChange> gol::SelectionManager::Deselect(GameGrid& grid)
{
    const auto retValue = [this, &grid]()
    {
        if (!m_Selected)
            return std::optional<VersionChange> {};

        auto insertedCells = grid.InsertGrid(*m_Selected, SelectionBounds().UpperLeft());
        return std::optional<VersionChange>
        {{
            .Action = SelectionAction::Deselect,
            .SelectionBounds = SelectionBounds(),
            .CellsInserted = insertedCells,
            .CellsDeleted = m_Selected->Data()
        }};
    }();

    m_LockSelection = true;
    m_AnchorSelection = std::nullopt;
    m_SentinelSelection = std::nullopt;
    m_Selected = std::nullopt;

    return retValue;
}

std::optional<gol::VersionChange> gol::SelectionManager::Copy(GameGrid& grid)
{
    if(!m_Selected)
        return std::nullopt;
    
    ImGui::SetClipboardText(RLEEncoder::EncodeRegion(*m_Selected, { { 0, 0 }, m_Selected->Size() }).c_str());
    return Deselect(grid);
}

std::optional<gol::VersionChange> gol::SelectionManager::Cut()
{
    if (!m_Selected)
		return std::nullopt;

    ImGui::SetClipboardText(RLEEncoder::EncodeRegion(*m_Selected, { 0, 0, m_Selected->Width(), m_Selected->Height() }).c_str());
    return Delete();
}

std::expected<gol::VersionChange, uint32_t> gol::SelectionManager::Paste(std::optional<Vec2> gridPos, uint32_t warnThreshold, bool unlock)
{
    if (unlock)
		m_LockSelection = false;

    if (!gridPos)
        gridPos = m_AnchorSelection;
    if (!gridPos)
        gridPos = { 0, 0 };

    auto decodeResult = RLEEncoder::DecodeRegion(ImGui::GetClipboardText(), warnThreshold);
    if (!decodeResult)
        return std::unexpected{ decodeResult.error() };

    m_Selected = std::move(decodeResult->Grid);
    m_SentinelSelection = gridPos;
    m_AnchorSelection = { gridPos->X + m_Selected->Width() - 1, gridPos->Y + m_Selected->Height() - 1 };

    return VersionChange
    {
        .Action = SelectionAction::Paste,
        .SelectionBounds = SelectionBounds(),
        .CellsInserted = m_Selected->Data(),
        .CellsDeleted = {}
    };
}

std::optional<gol::VersionChange> gol::SelectionManager::Delete()
{
    if (!m_Selected)
        return std::nullopt;

    auto bounds = SelectionBounds();
    auto retValue = VersionChange
    {
        .Action = SelectionAction::Delete,
        .SelectionBounds = SelectionBounds(),
        .CellsInserted = {},
        .CellsDeleted = m_Selected->Data()
    };

    m_Selected = std::nullopt;
    m_AnchorSelection = std::nullopt;
    m_SentinelSelection = std::nullopt;

    return retValue;
}

std::optional<gol::VersionChange> gol::SelectionManager::Rotate(bool clockwise)
{
    if (!m_Selected)
        return std::nullopt;

    auto upperLeft = SelectionBounds().UpperLeft();
    auto width = static_cast<float>(m_Selected->Width());
    auto height = static_cast<float>(m_Selected->Height());

    auto gridCenter = Vec2F
    {
        static_cast<float>(upperLeft.X) + width / 2.f,
        static_cast<float>(upperLeft.Y) + height / 2.f
    };

    auto oldCorner = Vec2F{ clockwise ? SelectionBounds().LowerLeft() : SelectionBounds().UpperRight() };
    m_AnchorSelection = RotatePoint(gridCenter, oldCorner, clockwise);
    m_SentinelSelection = *m_AnchorSelection + Vec2{ m_Selected->Height() - 1, m_Selected->Width() - 1 };
    m_Selected->RotateGrid(clockwise);

    return VersionChange
    {
        .Action = SelectionAction::Rotate,
        .SelectionBounds = SelectionBounds(),
    };
}

std::optional<gol::VersionChange> gol::SelectionManager::Flip(SelectionAction direction)
{
    if (!m_Selected)
		return std::nullopt;

	m_Selected->FlipGrid(direction == SelectionAction::FlipVertically);

    return VersionChange { .Action = direction };
}


std::optional<gol::VersionChange> gol::SelectionManager::Nudge(Vec2 translation)
{
    if (!m_AnchorSelection || (m_AnchorSelection == m_SentinelSelection))
        return std::nullopt;

    *m_AnchorSelection += translation;
    *m_SentinelSelection += translation;

    auto action = [translation]()
    {
        if (translation.X < 0)
            return SelectionAction::NudgeLeft;
        else if (translation.X > 0)
            return SelectionAction::NudgeRight;
        else if (translation.Y < 0)
            return SelectionAction::NudgeUp;
        return SelectionAction::NudgeDown;
    }();

    return VersionChange
    {
        .Action = action,
        .SelectionBounds = SelectionBounds(),
        .NudgeTranslation = translation
    };
}

std::expected<gol::VersionChange, std::string> gol::SelectionManager::Load(const std::filesystem::path& filePath)
{
	auto result = RLEEncoder::ReadRegion(filePath);
    if (!result)
        return std::unexpected { std::move(result.error()) };

    m_Selected = std::move(result->Grid);
	m_AnchorSelection = result->Offset;
	m_SentinelSelection = result->Offset + Vec2 { m_Selected->Width() - 1, m_Selected->Height() - 1 };

    return VersionChange
    {
        .Action = SelectionAction::Paste,
        .SelectionBounds = SelectionBounds(),
        .CellsInserted = m_Selected->Data(),
        .CellsDeleted = {}
	};

}

bool gol::SelectionManager::Save(const GameGrid& grid, const std::filesystem::path& filePath) const
{
    if (!m_Selected)
    {
		auto boundingBox = grid.BoundingBox();
        return RLEEncoder::WriteRegion(grid, boundingBox, filePath, boundingBox.Pos());

    }
    return RLEEncoder::WriteRegion(*m_Selected, Rect { { 0, 0 }, m_Selected->Size() }, filePath);
}

std::optional<gol::VersionChange> gol::SelectionManager::HandleAction(SelectionAction action, GameGrid& grid, int32_t nudgeSize)
{
    switch (action)
    {
    using enum SelectionAction;
    case Copy:       return this->Copy(grid);
    case Cut:        return this->Cut();
    case Delete:     return this->Delete();
    case Deselect:   return this->Deselect(grid);
    case NudgeLeft:  return Nudge({ -nudgeSize, 0 });
    case NudgeRight: return Nudge({ nudgeSize, 0 });
    case NudgeUp:    return Nudge({ 0, -nudgeSize });
    case NudgeDown:  return Nudge({ 0, nudgeSize });
    case Rotate:     return this->Rotate(true);
    case FlipVertically: [[fallthrough]];
	case FlipHorizontally: return this->Flip(action);
	default: assert(false && "Invalid action passed to HandleAction");
    }
    return std::nullopt;
}

void gol::SelectionManager::HandleVersionChange(EditorAction undoRedo, GameGrid& grid, const VersionChange& change)
{
    if (!change.Action)
    {
        RestoreGridVersion(undoRedo, grid, change);
        return;
    }

    m_UnlockedOriginalPosition = std::nullopt;

    if (auto* editorAction = std::get_if<EditorAction>(&*change.Action))
    {
        if (!change.GridResize)
            return;

        switch (*editorAction)
        {
        case EditorAction::Resize:
            if (undoRedo == EditorAction::Undo)
                grid = change.GridResize->first;
            else
                grid = GameGrid(grid, change.GridResize->second);
            return;
        default:
			assert(false && "Invalid EditorAction passed to HandleVersionChange");
        }
    }

    if (auto* gameAction = std::get_if<GameAction>(&*change.Action))
    {
        switch (*gameAction)
        {
        case GameAction::Clear:
            if (undoRedo == EditorAction::Undo)
            {
                for (auto pos : change.CellsDeleted)
                    grid.Set(pos.X, pos.Y, true);
            }
            else
            {
                for (auto pos : change.CellsDeleted)
                    grid.Set(pos.X, pos.Y, false);
            }
            return;
        default:
			assert(false && "Invalid GameAction passed to HandleVersionChange");
		}
    }

    auto* action = std::get_if<SelectionAction>(&*change.Action);
    if (!action)
        return;

    switch (*action)
    {
    using enum SelectionAction;
    case NudgeLeft: [[fallthrough]];
    case NudgeRight: [[fallthrough]];
    case NudgeUp: [[fallthrough]];
    case NudgeDown:
        if (undoRedo == EditorAction::Undo)
            Nudge(-change.NudgeTranslation);
        else
            Nudge(change.NudgeTranslation);
        return;
    case FlipHorizontally: [[fallthrough]];
    case FlipVertically:
		Flip(*action);
        return;
    case Rotate:
        this->Rotate(undoRedo == EditorAction::Redo);
        return;
    case Paste: [[fallthrough]];
    case Delete:
        SetSelectionBounds(*change.SelectionBounds);
        m_Selected = GameGrid(change.SelectionBounds->Size());
        RestoreGridVersion(undoRedo, *m_Selected, change);

        if ((undoRedo == EditorAction::Redo) == (*action == Delete))
            this->Deselect(grid);
        return;
    case Select:
        if (undoRedo == EditorAction::Undo)
        {
            this->Deselect(grid);
            return;
        }

        SetSelectionBounds(*change.SelectionBounds);
        m_Selected = GameGrid(change.SelectionBounds->Size());

        for (auto pos : change.CellsInserted)
            m_Selected->Set(pos.X, pos.Y, true);
        for (auto pos : change.CellsDeleted)
            grid.Set(pos.X, pos.Y, false);
        return;
    case Deselect:
        if (undoRedo == EditorAction::Undo)
        {
            SetSelectionBounds(*change.SelectionBounds);
            m_Selected = GameGrid(change.SelectionBounds->Size());
            for (auto pos : change.CellsDeleted)
                m_Selected->Set(pos.X, pos.Y, true);
            for (auto pos : change.CellsInserted)
                grid.Set(pos.X, pos.Y, false);
            return;
        }
        
        this->Deselect(grid);
        return;
    default:
		assert(false && "Invalid action passed to HandleVersionChange");
    }
}

void gol::SelectionManager::RestoreGridVersion(EditorAction undoRedo, GameGrid& grid, const VersionChange& change)
{
    auto& insertions = undoRedo == EditorAction::Undo
        ? change.CellsDeleted
        : change.CellsInserted;
    for (auto& pos : insertions)
        grid.Set(pos.X, pos.Y, true);

    auto& deletions = undoRedo == EditorAction::Undo
        ? change.CellsInserted
        : change.CellsDeleted;
    for (auto& pos : deletions)
        grid.Set(pos.X, pos.Y, false);
}

gol::Rect gol::SelectionManager::SelectionBounds() const
{
    return
    {
        std::min(m_AnchorSelection->X, m_SentinelSelection->X),
        std::min(m_AnchorSelection->Y, m_SentinelSelection->Y),
        std::abs(m_SentinelSelection->X - m_AnchorSelection->X) + 1,
        std::abs(m_SentinelSelection->Y - m_AnchorSelection->Y) + 1
    };
}

bool gol::SelectionManager::GridAlive() const
{
    return m_Selected && m_Selected->Population() > 0;
}

const gol::LifeHashSet& gol::SelectionManager::GridData() const
{
    return m_Selected->Data();
}

int64_t gol::SelectionManager::SelectedPopulation() const
{
    return m_Selected ? m_Selected->Population() : 0;
}

bool gol::SelectionManager::CanDrawSelection() const
{
    return m_AnchorSelection && m_SentinelSelection;
}

bool gol::SelectionManager::CanDrawLargeSelection() const
{
    return m_AnchorSelection && m_SentinelSelection && (*m_AnchorSelection != *m_SentinelSelection);
}

bool gol::SelectionManager::CanDrawGrid() const
{
    return m_Selected.has_value();
}

void gol::SelectionManager::SetSelectionBounds(const Rect& bounds)
{
    m_AnchorSelection = bounds.Pos();
    m_SentinelSelection = *m_AnchorSelection + Vec2
    {
        bounds.Width - 1,
        bounds.Height - 1
    };
}

gol::Vec2 gol::SelectionManager::RotatePoint(Vec2F center, Vec2F point, bool clockwise)
{
    auto offset = Vec2F{ static_cast<float>(point.X), static_cast<float>(point.Y) } - center;
    auto rotated = clockwise
        ? Vec2F{ -offset.Y,  offset.X }
    : Vec2F{ offset.Y, -offset.X };
    auto result = rotated + center;

    auto retValue = Vec2
    {
        static_cast<int32_t>(m_RotationParity ? std::floor(result.X) : std::ceil(result.X)),
        static_cast<int32_t>(!m_RotationParity ? std::floor(result.Y) : std::ceil(result.Y))
    };
    m_RotationParity = !m_RotationParity;
    return retValue;
}