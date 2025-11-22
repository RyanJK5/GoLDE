#include <algorithm>
#include <cmath>
#include <cstdint>
#include <imgui/imgui.h>
#include <optional>
#include <set>

#include "GameEnums.h"
#include "GameGrid.h"
#include "Graphics2D.h"
#include "RLEEncoder.h"
#include "SelectionManager.h"
#include "VersionManager.h"

gol::SelectionUpdateResult gol::SelectionManager::UpdateSelectionArea(GameGrid& grid, Vec2 gridPos)
{
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)))
    {
        m_SentinelSelection = gridPos;
        if (!m_AnchorSelection)
            m_AnchorSelection = gridPos;
        return { .BeginSelection = true };
    }

    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) && (m_AnchorSelection != m_SentinelSelection))
    {
        if (m_Selected)
            return { .BeginSelection = false };
    
        m_Selected = grid.SubRegion(SelectionBounds());
        auto change = VersionChange
        ({
            .Action = GameAction::Select,
            .SelectionBounds = SelectionBounds(),
            .CellsInserted = m_Selected->Data(),
            .CellsDeleted = grid.ReadRegion(SelectionBounds())
        });
        grid.ClearRegion(SelectionBounds());

        return { .Change = change, .BeginSelection = false };
    }

    Deselect(grid);
    m_AnchorSelection = gridPos;
    m_SentinelSelection = gridPos;
    return { .BeginSelection = false };
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
            .Action = GameAction::Deselect,
            .SelectionBounds = SelectionBounds(),
            .CellsInserted = insertedCells,
            .CellsDeleted = m_Selected->Data()
        }};
    }();

    m_AnchorSelection = std::nullopt;
    m_SentinelSelection = std::nullopt;
    m_Selected = std::nullopt;

    return retValue;
}

std::optional<gol::VersionChange> gol::SelectionManager::Copy(GameGrid& grid)
{
    if (!m_Selected)
        return std::nullopt;

    ImGui::SetClipboardText(
        reinterpret_cast<const char*>(
            RLEEncoder::EncodeRegion<uint32_t>(*m_Selected, { 0, 0, m_Selected->Width(), m_Selected->Height() }).data()
            )
    );

    return Deselect(grid);
}

std::optional<gol::VersionChange> gol::SelectionManager::Paste(GameGrid& grid, std::optional<Vec2> gridPos)
{
    if (!gridPos)
        gridPos = m_AnchorSelection;
    if (!gridPos)
        return std::nullopt;

    m_Selected = RLEEncoder::DecodeRegion<uint32_t>(ImGui::GetClipboardText());
    m_AnchorSelection = gridPos;
    m_SentinelSelection = { gridPos->X + m_Selected->Width() - 1, gridPos->Y + m_Selected->Height() - 1 };

    return VersionChange
    {
        .Action = GameAction::Paste,
        .SelectionBounds = SelectionBounds(),
        .CellsInserted = m_Selected->Data(),
        .CellsDeleted = {}
    };
}

std::optional<gol::VersionChange> gol::SelectionManager::Delete()
{
    return Delete(false);
}

std::optional<gol::VersionChange> gol::SelectionManager::Cut()
{
    return Delete(true);
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
        .Action = GameAction::Rotate,
        .SelectionBounds = SelectionBounds(),
    };
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
            return GameAction::NudgeLeft;
        else if (translation.X > 0)
            return GameAction::NudgeRight;
        else if (translation.Y < 0)
            return GameAction::NudgeUp;
        return GameAction::NudgeDown;
    }();

    return VersionChange
    {
        .Action = action,
        .SelectionBounds = SelectionBounds(),
        .NudgeTranslation = translation
    };
}

void gol::SelectionManager::HandleVersionChange(GameAction undoRedo, GameGrid& grid, const VersionChange& change)
{
    switch (change.Action)
    {
    using enum GameAction;
    case None:
        RestoreGridVersion(undoRedo, grid, change);
        return;
    case NudgeLeft: [[fallthrough]];
    case NudgeRight: [[fallthrough]];
    case NudgeUp: [[fallthrough]];
    case NudgeDown:
        if (undoRedo == Undo)
            Nudge(-change.NudgeTranslation);
        else
            Nudge(change.NudgeTranslation);
        return;
    case Rotate:
        this->Rotate(undoRedo == Redo);
        return;
    case Paste: [[fallthrough]];
    case Delete:
        SetSelectionBounds(*change.SelectionBounds);
        m_Selected = GameGrid(change.SelectionBounds->Size());
        RestoreGridVersion(undoRedo, *m_Selected, change);

        if ((undoRedo == Redo) == (change.Action == Delete))
            this->Deselect(grid);
        return;
    case Select:
        if (undoRedo == Undo)
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
        if (undoRedo == Undo)
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
    }
}

void gol::SelectionManager::RestoreGridVersion(GameAction undoRedo, GameGrid& grid, const VersionChange& change)
{
    auto& insertions = undoRedo == GameAction::Undo
        ? change.CellsDeleted
        : change.CellsInserted;
    for (auto& pos : insertions)
        grid.Set(pos.X, pos.Y, true);

    auto& deletions = undoRedo == GameAction::Undo
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

const std::set<gol::Vec2>& gol::SelectionManager::GridData() const
{
    return m_Selected->Data();
}

int32_t gol::SelectionManager::SelectedPopulation() const
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

std::optional<gol::VersionChange> gol::SelectionManager::Delete(bool cut)
{
    if (!m_Selected)
        return std::nullopt;

    auto bounds = SelectionBounds();
    auto retValue = VersionChange
    {
        .Action = GameAction::Delete,
        .SelectionBounds = SelectionBounds(),
        .CellsInserted = {},
        .CellsDeleted = m_Selected->Data()
    };

    if (cut)
        ImGui::SetClipboardText(
            reinterpret_cast<const char*>(
                RLEEncoder::EncodeRegion<uint32_t>(*m_Selected, { 0, 0, m_Selected->Width(), m_Selected->Height() }).data()
                )
        );

    m_Selected = std::nullopt;
    m_AnchorSelection = std::nullopt;
    m_SentinelSelection = std::nullopt;

    return retValue;
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