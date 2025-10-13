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
    , m_Graphics(GraphicsHandler("shader/default.shader", DefaultWindowWidth, DefaultWindowHeight))
{ }

void gol::Game::UpdateState()
{
    bool enterState = m_Window.GetKeyState(ImGuiKey_Enter);
    if (enterState)
        m_Input.EnterDown = true;
    else if (m_Input.EnterDown && !enterState)
    {
        m_Input.EnterDown = false;
        switch (m_Settings.State)
        {
        case GameState::Paint:
            m_Settings.State = GameState::Simulation;
            return;
        case GameState::Simulation:
            m_Grid = GameGrid(m_Grid.Size());
            m_Settings.State = GameState::Paint;
            break;
        }
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
        if (m_Input.DrawMode == DrawMode::None)
            m_Input.DrawMode = *m_Grid.Get(gridPos.X, gridPos.Y) ? DrawMode::Delete : DrawMode::Insert;

        m_Grid.Set(gridPos.X, gridPos.Y, m_Input.DrawMode == DrawMode::Insert);
    }
    else
        m_Input.DrawMode = DrawMode::None;
}

static double GetTimeMs(const std::chrono::steady_clock& clock)
{
    return clock.now().time_since_epoch().count() / pow(10, 6);
}

bool gol::Game::SimulationUpdate(double timeElapsedMs)
{   
    const bool success = timeElapsedMs >= m_Settings.TickDelayMs;
    if (success)
    {
        if (m_Grid.Dead())
        {
            m_Settings.State = GameState::Paint;
            m_Grid = GameGrid(DefaultGridWidth, DefaultGridHeight);
        }
        else
            m_Grid.Update();
    }

    m_Graphics.ClearBackground(m_Window.WindowBounds(), m_Window.ViewportBounds(m_Grid.Size()));
    m_Graphics.DrawGrid(m_Grid.GenerateGLBuffer());
    
    return success;
}

void gol::Game::PaintUpdate()
{
    m_Graphics.ClearBackground(m_Window.WindowBounds(), m_Window.ViewportBounds(m_Grid.Size()));
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

void gol::Game::Begin()
{
    const std::chrono::steady_clock clock{};
    double lastTimeMs = GetTimeMs(clock);

    while (m_Window.Open())
    {
        m_Window.FrameStart(m_Graphics.TextureID());
        m_Window.UpdateViewport(m_Grid.Size());
        m_Graphics.RescaleFrameBuffer(m_Window.WindowBounds().Size());
        UpdateState();

        switch (m_Settings.State)
        {
        case GameState::Paint:
            PaintUpdate();
            break;
        case GameState::Simulation:
            double currentTimeMs = GetTimeMs(clock);
            if (SimulationUpdate(currentTimeMs - lastTimeMs))
                lastTimeMs = currentTimeMs;
            break;
        }

        m_Window.FrameEnd();
    }
}