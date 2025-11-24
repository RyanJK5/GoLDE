#include <cmath>
#include <cstdint>
#include <filesystem>
#include <format>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <imgui/imgui.h>
#include <locale>
#include <optional>
#include <string>
#include <utility>
#include <variant>

#include "GameEnums.h"
#include "GameGrid.h"
#include "GraphicsHandler.h"
#include "Graphics2D.h"
#include "Logging.h"
#include "SimulationControlResult.h"
#include "SimulationEditor.h"
#include "VersionManager.h"
#include "WarnWindow.h"

gol::SimulationEditor::SimulationEditor(Size2 windowSize, Size2 gridSize)
    : m_Grid(gridSize)
    , m_Graphics(std::filesystem::path("resources") / "shader" / "default.shader", windowSize.Width, windowSize.Height)
    , m_PasteWarning("Warning")
{ }   

gol::SimulationState gol::SimulationEditor::Update(const SimulationControlResult& args)
{
    auto graphicsArgs = GraphicsHandlerArgs { .ViewportBounds = ViewportBounds(), .GridSize = m_Grid.Size() };

    auto pasteWarnResult = m_PasteWarning.Update();
    if (pasteWarnResult == WarnWindowState::Success)
    {
        auto pasteResult = m_SelectionManager.Paste(CursorGridPos(), std::numeric_limits<uint32_t>::max());
        if (pasteResult)
            m_VersionManager.PushChange(*pasteResult);
        m_PasteWarning.Active = false;
    }
    else if (pasteWarnResult == WarnWindowState::Failure)
        m_PasteWarning.Active = false;

    UpdateViewport();
    UpdateDragState();
    m_Graphics.RescaleFrameBuffer(WindowBounds().Size());
    m_Graphics.ClearBackground(graphicsArgs);
    
    if (args.TickDelayMs)
        m_TickDelayMs = *args.TickDelayMs;

    SimulationState state = !args.Action
        ? args.State 
        : UpdateState(args);

    state = [this, state, &graphicsArgs]()
    {
        switch (state)
        {
        using enum SimulationState;
        case Simulation:
            return SimulationUpdate(graphicsArgs);
        case Paint:
            return PaintUpdate(graphicsArgs);
        case Paused:
            return PauseUpdate(graphicsArgs);
        case Empty:
            return PaintUpdate(graphicsArgs);
        };
        std::unreachable();
    }();

    DisplaySimulation();
    return state;
}

gol::SimulationState gol::SimulationEditor::SimulationUpdate(const GraphicsHandlerArgs& args)
{
    GL_DEBUG(const bool success = glfwGetTime() * 1000 >= m_TickDelayMs);
    if (success)
    {
        GL_DEBUG(glfwSetTime(0));
        m_Grid.Update();
        if (m_Grid.Dead() && !m_SelectionManager.GridAlive())
            return SimulationState::Empty;
    }
    m_Graphics.DrawGrid({ 0, 0 }, m_Grid.Data(), args);
    return SimulationState::Simulation;
}

gol::SimulationState gol::SimulationEditor::PaintUpdate(const GraphicsHandlerArgs& args)
{
    auto gridPos = CursorGridPos();
    if (m_SelectionManager.CanDrawGrid())
        m_Graphics.DrawSelection(m_SelectionManager.SelectionBounds(), args);
    
    m_Graphics.DrawGrid({ 0, 0 }, m_Grid.Data(), args);
    if (m_SelectionManager.CanDrawGrid())
        m_Graphics.DrawGrid(m_SelectionManager.SelectionBounds().UpperLeft(), m_SelectionManager.GridData(), args);
    
    if (gridPos)
    {
        if (m_SelectionManager.CanDrawSelection())
            m_Graphics.DrawSelection(m_SelectionManager.SelectionBounds(), args);
        UpdateMouseState(*gridPos);
    }
    else
    {
        m_SelectionManager.TryResetSelection();
    }

    return (m_Grid.Dead() && !m_SelectionManager.GridAlive())
        ? SimulationState::Empty 
        : SimulationState::Paint;
}

gol::SimulationState gol::SimulationEditor::PauseUpdate(const GraphicsHandlerArgs& args)
{
    auto gridPos = CursorGridPos();
    if (gridPos)
        m_VersionManager.TryPushChange(m_SelectionManager.UpdateSelectionArea(m_Grid, *gridPos).Change);
    if (m_SelectionManager.CanDrawSelection())
        m_Graphics.DrawSelection(m_SelectionManager.SelectionBounds(), args);
    m_Graphics.DrawGrid({ 0, 0 }, m_Grid.Data(), args);
    if (m_SelectionManager.CanDrawGrid())
        m_Graphics.DrawGrid(m_SelectionManager.SelectionBounds().UpperLeft(), m_SelectionManager.GridData(), args);
    return SimulationState::Paused;
}

void gol::SimulationEditor::DisplaySimulation()
{
    ImGui::Begin("Simulation");
    ImGui::BeginChild("Render");

    ImDrawListSplitter splitter {};
    splitter.Split(ImGui::GetWindowDrawList(), 2);

    splitter.SetCurrentChannel(ImGui::GetWindowDrawList(), 0);
    m_WindowBounds = { Vec2F(ImGui::GetWindowPos()), Size2F(ImGui::GetContentRegionAvail()) };

    ImGui::Image(
        static_cast<ImTextureID>(m_Graphics.TextureID()),
        ImGui::GetContentRegionAvail(),
        ImVec2(0, 1),
        ImVec2(1, 0)
    );

    splitter.SetCurrentChannel(ImGui::GetWindowDrawList(), 1);
    ImGui::SetCursorPos(ImGui::GetWindowContentRegionMin());
    ImGui::Text(std::format("Generation: {}", m_Grid.Generation()).c_str());
    ImGui::Text(std::format("Population: {}", m_Grid.Population() + m_SelectionManager.SelectedPopulation()).c_str());

    if (m_SelectionManager.CanDrawSelection())
    {
        Vec2 pos = m_SelectionManager.SelectionBounds().UpperLeft();
        std::string text = std::format("({}, {})", pos.X, pos.Y);
        if (m_SelectionManager.CanDrawLargeSelection())
        {
            auto sentinel = m_SelectionManager.SelectionBounds().LowerRight();
            text += std::format(" X ({}, {})", sentinel.X, sentinel.Y);
        }
        ImGui::SetCursorPosY(ImGui::GetContentRegionMax().y - ImGui::CalcTextSize(text.c_str()).y);
        ImGui::Text(text.c_str());
    }

    splitter.Merge(ImGui::GetWindowDrawList());
    ImGui::EndChild();
    ImGui::End();
}

gol::Rect gol::SimulationEditor::WindowBounds() const
{
    return Rect(
        static_cast<int32_t>(m_WindowBounds.X),
        static_cast<int32_t>(m_WindowBounds.Y),
        static_cast<int32_t>(m_WindowBounds.Width),
        static_cast<int32_t>(m_WindowBounds.Height)
    );
}

gol::Rect gol::SimulationEditor::ViewportBounds() const
{
    return WindowBounds();
}

std::optional<gol::Vec2> gol::SimulationEditor::CursorGridPos()
{
    Vec2F cursor = ImGui::GetMousePos();
    if (!ViewportBounds().InBounds(cursor.X, cursor.Y))
        return std::nullopt;

    glm::vec2 vec = m_Graphics.Camera.ScreenToWorldPos(cursor, ViewportBounds());
    glm::vec2 other = m_Graphics.Camera.ScreenToWorldPos(Vec2F{ (float)ViewportBounds().X, (float)ViewportBounds().Y }, ViewportBounds());
    vec /= glm::vec2 { DefaultCellWidth  , 
                       DefaultCellHeight };
    
    Vec2 result = { static_cast<int32_t>(std::floor(vec.x)), static_cast<int32_t>(std::floor(vec.y)) };
    if (!m_Grid.InBounds(result))
        return std::nullopt;
    return result;
}

void gol::SimulationEditor::UpdateVersion(const SimulationControlResult& args)
{
	auto action = std::get<EditorAction>(*args.Action);

    auto versionChanges = action == EditorAction::Undo
        ? m_VersionManager.Undo()
		: m_VersionManager.Redo();
    if (!versionChanges)
        return;

	m_SelectionManager.HandleVersionChange(action, m_Grid, *versionChanges);
}

gol::SimulationState gol::SimulationEditor::UpdateState(const SimulationControlResult& result)
{
    if (auto* action = std::get_if<GameAction>(&*result.Action))
    {
        switch(*action)
        {
        using enum GameAction;
        case Start:
            m_VersionManager.TryPushChange(m_SelectionManager.Deselect(m_Grid));
            m_InitialGrid = m_Grid;
            return SimulationState::Simulation;
        case Clear:
            m_VersionManager.TryPushChange(m_SelectionManager.Deselect(m_Grid));
            m_Grid = GameGrid(m_Grid.Size());
            return SimulationState::Paint;
        case Reset:
            m_VersionManager.TryPushChange(m_SelectionManager.Deselect(m_Grid));
            m_Grid = m_InitialGrid;
            return SimulationState::Paint;
        case Restart:
            m_VersionManager.TryPushChange(m_SelectionManager.Deselect(m_Grid));
            m_Grid = m_InitialGrid;
            return SimulationState::Simulation;
        case Pause:
            return SimulationState::Paused;
        case Resume:
            m_VersionManager.TryPushChange(m_SelectionManager.Deselect(m_Grid));
            return SimulationState::Simulation;
        case Step:
            m_VersionManager.TryPushChange(m_SelectionManager.Deselect(m_Grid));
            for (int32_t i = 0; i < *result.StepCount; i++)
                m_Grid.Update();
            return m_Grid.Dead() ? SimulationState::Empty : SimulationState::Paused;
        }
    }

    if (auto* action = std::get_if<EditorAction>(&*result.Action))
    {
        switch (*action)
        {
        using enum EditorAction;
        case Resize:
            return ResizeGrid(result);
        case Undo:
            UpdateVersion(result);
            return result.State;
        case Redo:
            UpdateVersion(result);
            return result.State;
        }
	}

    if (auto* action = std::get_if<SelectionAction>(&*result.Action))
    {
        if (*action == SelectionAction::Paste)
        {
            auto gridPos = CursorGridPos();
            if (gridPos)
                m_VersionManager.TryPushChange(m_SelectionManager.Deselect(m_Grid));
            auto pasteResult = m_SelectionManager.Paste(gridPos, 100000U);
            if (pasteResult)
				m_VersionManager.TryPushChange(*pasteResult);
            else if (pasteResult.error() != 0)
            {
                m_PasteWarning.Active = true;
                m_PasteWarning.Message = std::format(
                    std::locale(""),
                    "Your selection ({:L} cells) is too large\n"
                    "to paste without potential performance issues.\n"
                    "Are you sure you want to continue?"
                    , pasteResult.error())
                ;
            }
            return result.State;
        }
        m_VersionManager.TryPushChange(m_SelectionManager.HandleAction(*action, m_Grid, result.NudgeSize));
    }

    return result.State;
}

gol::SimulationState gol::SimulationEditor::ResizeGrid(const gol::SimulationControlResult& result)
{
    m_VersionManager.PushChange
    ({
        .Action = EditorAction::Resize,
        .GridResize = { { m_Grid, *result.NewDimensions } }
        });

    m_Grid = GameGrid(std::move(m_Grid), *result.NewDimensions);
    if (m_SelectionManager.CanDrawSelection())
    {
        auto selection = m_SelectionManager.SelectionBounds();
        if (!m_Grid.InBounds(selection.UpperLeft()) || !m_Grid.InBounds(selection.UpperRight()) ||
            !m_Grid.InBounds(selection.LowerLeft()) || !m_Grid.InBounds(selection.LowerRight()))
            m_VersionManager.TryPushChange(m_SelectionManager.Deselect(m_Grid));
    }
    m_Graphics.Camera.Center =
    {
        result.NewDimensions->Width * DefaultCellWidth / 2.f,
        result.NewDimensions->Height * DefaultCellHeight / 2.f
    };
    return SimulationState::Paint;
}

void gol::SimulationEditor::UpdateMouseState(Vec2 gridPos)
{
    if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId))
        return;

    auto result = m_SelectionManager.UpdateSelectionArea(m_Grid, gridPos);
    if (result.BeginSelection)
    {
        m_EditorMode = EditorMode::Select;
        return;
    }
    m_VersionManager.TryPushChange(result.Change);

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && !ImGui::IsKeyDown(ImGuiKey_LeftShift) && !ImGui::IsKeyDown(ImGuiKey_RightShift))
    {
        if (m_EditorMode == EditorMode::None)
        {
            if (m_SelectionManager.CanDrawLargeSelection())
            {
				m_SelectionManager.Deselect(m_Grid);
				m_SelectionManager.UpdateSelectionArea(m_Grid, gridPos);
            }

            m_EditorMode = *m_Grid.Get(gridPos.X, gridPos.Y) ? EditorMode::Delete : EditorMode::Insert;
            m_VersionManager.BeginPaintChange(gridPos, m_EditorMode == EditorMode::Insert);
        }
        if (*m_Grid.Get(gridPos.X, gridPos.Y) != (m_EditorMode == EditorMode::Insert))
			m_VersionManager.AddPaintChange(gridPos);
        m_Grid.Set(gridPos.X, gridPos.Y, m_EditorMode == EditorMode::Insert);
        return;
    }
    m_EditorMode = EditorMode::None;
}

void gol::SimulationEditor::UpdateDragState()
{
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
    {
        Vec2F delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
        m_Graphics.Camera.Translate(glm::vec2(delta) - m_DeltaLast);
        m_DeltaLast = delta;
    }
    else
    {
        m_DeltaLast = glm::vec2(0.f);
    }
}

void gol::SimulationEditor::UpdateViewport()
{
    const Rect bounds = ViewportBounds();
    
    auto mousePos = ImGui::GetIO().MousePos;
    if (bounds.InBounds(mousePos.x, mousePos.y) && ImGui::GetIO().MouseWheel != 0)
        m_Graphics.Camera.ZoomBy(mousePos, bounds, ImGui::GetIO().MouseWheel / 10.f);

    glViewport(static_cast<int32_t>(bounds.X - m_WindowBounds.X), static_cast<int32_t>(bounds.Y - m_WindowBounds.Y), bounds.Width, bounds.Height);
}