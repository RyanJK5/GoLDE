#include <exception>
#include <cmath>
#include <format>
#include <utility>

#include "RLEEncoder.h"
#include "SimulationEditor.h"
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "GameEnums.h"
#include "GameGrid.h"
#include "Graphics2D.h"
#include "GraphicsHandler.h"
#include "SimulationControlResult.h"
#include "glm/fwd.hpp"
#include "imgui.h"
#include "VersionManager.h"
#include <vector>
#include <array>

gol::SimulationEditor::SimulationEditor(Size2 windowSize, Size2 gridSize)
    : m_Grid(gridSize)
    , m_Graphics(std::filesystem::path("resources") / "shader" / "default.shader", windowSize.Width, windowSize.Height)
{ }   

gol::GameState gol::SimulationEditor::Update(const SimulationControlResult& args)
{
    GraphicsHandlerArgs graphicsArgs = { .ViewportBounds = ViewportBounds(), .GridSize = m_Grid.Size() };

    UpdateViewport();
    UpdateDragState();
    m_Graphics.RescaleFrameBuffer(WindowBounds().Size());
    m_Graphics.ClearBackground(graphicsArgs);
    

    if (args.TickDelayMs)
        m_TickDelayMs = *args.TickDelayMs;

    GameState state = args.Action == GameAction::None 
        ? args.State 
        : UpdateState(args);

    state = [&]()
    {
        switch (state)
        {
        using enum GameState;
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

gol::GameState gol::SimulationEditor::SimulationUpdate(const GraphicsHandlerArgs& args)
{
    const bool success = glfwGetTime() * 1000 >= m_TickDelayMs;
    if (success)
    {
        glfwSetTime(0);
        m_Grid.Update();
        if (m_Grid.Dead() && !m_SelectionManager.GridAlive())
            return GameState::Empty;
    }
    m_Graphics.DrawGrid({ 0, 0 }, m_Grid.Data(), args);
    return GameState::Simulation;
}

gol::GameState gol::SimulationEditor::PaintUpdate(const GraphicsHandlerArgs& args)
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

    return (m_Grid.Dead() && !m_SelectionManager.GridAlive())
        ? GameState::Empty 
        : GameState::Paint;
}

gol::GameState gol::SimulationEditor::PauseUpdate(const GraphicsHandlerArgs& args)
{
    auto gridPos = CursorGridPos();
    if (gridPos)
        m_VersionManager.TryPushChange(m_SelectionManager.UpdateSelectionArea(m_Grid, *gridPos).Change);
    if (m_SelectionManager.CanDrawSelection())
        m_Graphics.DrawSelection(m_SelectionManager.SelectionBounds(), args);
    m_Graphics.DrawGrid({ 0, 0 }, m_Grid.Data(), args);
    if (m_SelectionManager.CanDrawGrid())
        m_Graphics.DrawGrid(m_SelectionManager.SelectionBounds().UpperLeft(), m_SelectionManager.GridData(), args);
    return GameState::Paused;
}

void gol::SimulationEditor::DisplaySimulation()
{
    ImGui::Begin("Simulation");
    ImGui::BeginChild("Render");

    ImDrawListSplitter splitter;
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
    auto versionChanges = args.Action == GameAction::Undo
        ? m_VersionManager.Undo()
		: m_VersionManager.Redo();
    if (!versionChanges)
        return;

	m_SelectionManager.HandleVersionChange(args.Action, m_Grid, *versionChanges);
}

gol::GameState gol::SimulationEditor::UpdateState(const SimulationControlResult& result)
{
    switch (result.Action)
    {
    using enum GameAction;
    case Start:
        m_VersionManager.TryPushChange(m_SelectionManager.Deselect(m_Grid));
        m_InitialGrid = m_Grid;
        return GameState::Simulation;
    case Clear:
        m_VersionManager.TryPushChange(m_SelectionManager.Deselect(m_Grid));
        m_Grid = GameGrid(m_Grid.Size());
        return GameState::Paint;
    case Reset:
        m_Grid = m_InitialGrid;
        return GameState::Paint;
    case Restart:
        m_Grid = m_InitialGrid;
        return GameState::Simulation;
    case Pause:
        return GameState::Paused;
    case Resume:
        m_VersionManager.TryPushChange(m_SelectionManager.Deselect(m_Grid));
        return GameState::Simulation;
    case Step:
        for (int32_t i = 0; i < *result.StepCount; i++)
            m_Grid.Update();
        return m_Grid.Dead() ? GameState::Empty : GameState::Paused;
    case Resize:
        m_Grid = GameGrid(m_Grid, *result.NewDimensions);
        m_Graphics.Camera.Center = { result.NewDimensions->Width * DefaultCellWidth / 2.f, result.NewDimensions->Height * DefaultCellHeight / 2.f };
        return GameState::Paint;
    case Copy:
        m_VersionManager.TryPushChange(m_SelectionManager.Copy(m_Grid));
        return result.State;
    case Cut:
        m_VersionManager.TryPushChange(m_SelectionManager.Cut());
        return result.State;
    case Paste:
        m_VersionManager.TryPushChange(m_SelectionManager.Deselect(m_Grid));
        m_VersionManager.TryPushChange(m_SelectionManager.Paste(m_Grid, CursorGridPos()));
        return result.State;
    case Delete:
        m_VersionManager.TryPushChange(m_SelectionManager.Delete());
        return result.State;
    case Deselect:
        m_VersionManager.TryPushChange(m_SelectionManager.Deselect(m_Grid));
        return result.State;
    case NudgeLeft:
        m_VersionManager.TryPushChange(m_SelectionManager.Nudge({ -result.NudgeSize, 0 }));
        return result.State;
    case NudgeRight:
        m_VersionManager.TryPushChange(m_SelectionManager.Nudge({ result.NudgeSize, 0 }));
        return result.State;
    case NudgeUp:
        m_VersionManager.TryPushChange(m_SelectionManager.Nudge({ 0, -result.NudgeSize }));
        return result.State;
    case NudgeDown:
        m_VersionManager.TryPushChange(m_SelectionManager.Nudge({ 0, result.NudgeSize }));
        return result.State;
    case Rotate:
        m_VersionManager.TryPushChange(m_SelectionManager.Rotate(true));
        return result.State;
    case Undo:
        UpdateVersion(result);
        return result.State;
    case Redo:
		UpdateVersion(result);
        return result.State;
    }

    throw std::exception("Cannot pass 'None' as action to UpdateState");
}

void gol::SimulationEditor::UpdateMouseState(Vec2 gridPos)
{
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