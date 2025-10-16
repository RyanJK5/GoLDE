#include <fstream>
#include <chrono>
#include <iostream>
#include <algorithm>

#include "vendor/imgui.h"
#include "vendor/imgui_impl_glfw.h"
#include "vendor/imgui_impl_opengl3.h"

#include "ShaderManager.h"
#include "GLException.h"
#include "Logging.h"
#include "Game.h"
#include "Graphics2D.h"

gol::Game::Game()
    : m_Window(DefaultWindowWidth, DefaultWindowHeight)
    , m_Graphics(GraphicsHandler(std::filesystem::path("resources") / "shader" / "default.shader", DefaultWindowWidth, DefaultWindowHeight))
{ }

void gol::Game::UpdateState(const UpdateInfo& info)
{
    switch (info.Action)
    {
    case GameAction::Start:
        m_State = GameState::Simulation;
        m_InitialGrid = m_Grid;
        return;
    case GameAction::Clear:
        m_Grid = GameGrid(m_Grid.Size());
        m_State = GameState::Paint;
        return;
    case GameAction::Reset:
        m_Grid = m_InitialGrid;
        m_State = GameState::Paint;
        return;
    case GameAction::Restart:
        m_Grid = m_InitialGrid;
        m_State = GameState::Simulation;
        return;
    case GameAction::Pause:
        m_State = GameState::Paused;
        return;
    case GameAction::Resume:
        m_State = GameState::Simulation;
        return;
    }
}

std::optional<gol::Vec2> gol::Game::CursorGridPos()
{
    Rect view = m_Window.ViewportBounds(m_Grid.Size());
    Vec2F cursor = m_Window.CursorPos();
    if (!view.InBounds(cursor.X, cursor.Y))
        return std::nullopt;

    int32_t xPos = static_cast<int32_t>((cursor.X - view.X) / (float(view.Width) / m_Grid.Width()));
    int32_t yPos = static_cast<int32_t>((cursor.Y - view.Y) / (float(view.Height) / m_Grid.Height()));
    if (xPos >= m_Grid.Width() || yPos >= m_Grid.Height())
        return std::nullopt;
    
    return Vec2(xPos, yPos);
}

void gol::Game::UpdateMouseState(Vec2 gridPos)
{
    bool mouseState = m_Window.GetMouseState(ImGuiMouseButton_Left);
    if (mouseState)
    {
        if (m_DrawMode == DrawMode::None)
            m_DrawMode = *m_Grid.Get(gridPos.X, gridPos.Y) ? DrawMode::Delete : DrawMode::Insert;

        m_Grid.Set(gridPos.X, gridPos.Y, m_DrawMode == DrawMode::Insert);
    }
    else
        m_DrawMode = DrawMode::None;
}

static double GetTimeMs(const std::chrono::steady_clock& clock)
{
    return clock.now().time_since_epoch().count() / pow(10, 6);
}

bool gol::Game::SimulationUpdate(double timeElapsedMs)
{   
    const bool success = timeElapsedMs >= m_TickDelayMs;
    if (success)
    {
        m_Grid.Update();
    }

    m_Graphics.DrawGrid(m_Grid.GenerateGLBuffer());
    return success;
}

void gol::Game::PaintUpdate()
{
    m_Graphics.DrawGrid(m_Grid.GenerateGLBuffer());

    const std::optional<Vec2> gridPos = CursorGridPos();
    if (gridPos)
    {
        UpdateMouseState(*gridPos);
        m_Graphics.DrawSelection({
            m_Grid.GLCoords(gridPos->X, gridPos->Y),
            m_Grid.GLCellDimensions()
        });
    }
}

void gol::Game::PauseUpdate()
{
    m_Graphics.DrawGrid(m_Grid.GenerateGLBuffer());
}

void gol::Game::Begin()
{
    const std::chrono::steady_clock clock{};
    double lastTimeMs = GetTimeMs(clock);

    while (m_Window.Open())
    {
        m_Window.BeginFrame();

        m_Window.UpdateViewport(m_Grid.Size());
        m_Graphics.RescaleFrameBuffer(m_Window.WindowBounds().Size());
        m_Graphics.ClearBackground(m_Window.WindowBounds(), m_Window.ViewportBounds(m_Grid.Size()));

        const UpdateInfo& info = m_Window.CreateGUI({ m_Graphics.TextureID(), m_State, m_Grid.Dead()});
        UpdateState(info);

        switch (m_State)
        {
        case GameState::Paint:
            PaintUpdate();
            break;
        case GameState::Paused:
            PauseUpdate();
            break;
        case GameState::Simulation:
            double currentTimeMs = GetTimeMs(clock);
            if (SimulationUpdate(currentTimeMs - lastTimeMs))
                lastTimeMs = currentTimeMs;
            break;
        }

        m_Window.EndFrame();
    }
}