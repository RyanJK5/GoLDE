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
    : m_window(nullptr)
    , m_draw()
    , m_grid(64, 64)
    , m_settings(GameState::Paint, 10)
    , m_input()
{
    try
    {
        InitOpenGL();
        InitImGUI();
    }
    catch (GLException e)
    {
        glfwTerminate();
        throw e;
    }
}

gol::Game::~Game()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}

gol::Rect gol::Game::WindowBounds() const
{
    int32_t windowWidth, windowHeight;
    glfwGetFramebufferSize(m_window, &windowWidth, &windowHeight);
    return { 0, 0, windowWidth, windowHeight };
}

void gol::Game::UpdateViewport()
{
    Rect bounds = ViewportBounds();
    glViewport(bounds.X, bounds.Y, bounds.Width, bounds.Height);
}

void gol::Game::UpdateState()
{
    int enterState = glfwGetKey(m_window, GLFW_KEY_ENTER);
    if (enterState == GLFW_PRESS)
        m_input.EnterDown = true;
    else if (m_input.EnterDown && enterState == GLFW_RELEASE)
    {
        m_input.EnterDown = false;
        switch (m_settings.State)
        {
        case GameState::Paint:
            m_settings.State = GameState::Simulation;
            return;
        case GameState::Simulation:
            m_grid = GameGrid(m_grid.Width(), m_grid.Height());
            m_settings.State = GameState::Paint;
            break;
        }
    }
}

std::optional<gol::Vec2> gol::Game::GetCursorGridPos()
{
    double x, y;
    glfwGetCursorPos(m_window, &x, &y);
    Rect view = ViewportBounds();
    if (!view.InBounds(x, y))
        return std::nullopt;

    uint32_t xPos = (x - view.X) / (float(view.Width) / m_grid.Width());
    uint32_t yPos = (y - view.Y) / (float(view.Height) / m_grid.Height());
    if (xPos < 0 || xPos >= m_grid.Width() || yPos < 0 || yPos >= m_grid.Height())
        return std::nullopt;
    
    return Vec2(xPos, yPos);
}

void gol::Game::UpdateMouseState(Vec2 gridPos)
{
    int mouseState = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT);
    if (mouseState == GLFW_PRESS)
    {
        if (m_input.DrawMode == DrawMode::None)
            m_input.DrawMode = m_grid.Get(gridPos.X, gridPos.Y) ? DrawMode::Delete : DrawMode::Insert;

        m_grid.Set(gridPos.X, gridPos.Y, m_input.DrawMode == DrawMode::Insert);
    }
    else if (mouseState == GLFW_RELEASE)
        m_input.DrawMode = DrawMode::None;
}

gol::Rect gol::Game::ViewportBounds() const
{
    int32_t windowWidth, windowHeight;
    glfwGetFramebufferSize(m_window, &windowWidth, &windowHeight);

    const float widthRatio = static_cast<float>(windowWidth) / m_grid.Width(); // 1920 / 64 = 30
    const float heightRatio = static_cast<float>(windowHeight) / m_grid.Height(); // 1080 / 64 = 16.875
    if (widthRatio > heightRatio)
    {
        const int32_t newWidth = heightRatio * m_grid.Width();
        const int32_t newX = (windowWidth - newWidth) / 2;
        return Rect{ newX, 0, newWidth, windowHeight };
    }
    const int32_t newHeight = widthRatio * m_grid.Height();
    const int32_t newY = (windowHeight - newHeight) / 2;
    return Rect{ 0, newY, windowWidth, newHeight };
}

void gol::Game::InitOpenGL()
{
    if (!glfwInit())
        throw GLException("Failed to initialize glfw");

    m_window = glfwCreateWindow(DefaultWindowWidth, DefaultWindowHeight, "Conway's Game of Life", NULL, NULL);

    if (!m_window)
        throw GLException("Failed to create window");

    glfwMakeContextCurrent(m_window);

    glLineWidth(4);

    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK)
        throw GLException("Failed to initialize glew");

    gol::ShaderManager shader("shader/default.shader");
    GL_DEBUG(glUseProgram(shader.Program()));
    
    UpdateViewport();
}

void gol::Game::InitImGUI()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init();
}

static double GetTimeMs(const std::chrono::steady_clock& clock)
{
    return clock.now().time_since_epoch().count() / pow(10, 6);
}

bool gol::Game::SimulationUpdate(double timeElapsedMs)
{   
    const bool success = timeElapsedMs >= m_settings.TickDelayMs;
    if (success)
    {
        if (m_grid.Dead())
        {
            m_settings.State = GameState::Paint;
            m_grid = GameGrid(DefaultGridWidth, DefaultGridHeight);
        }
        else
            m_grid.Update();
    }

    m_draw.ClearBackground(WindowBounds(), ViewportBounds());
    m_draw.DrawGrid(m_grid.GenerateGLBuffer());
    
    return success;
}

void gol::Game::PaintUpdate()
{
    m_draw.ClearBackground(WindowBounds(), ViewportBounds());
    m_draw.DrawGrid(m_grid.GenerateGLBuffer());

    const std::optional<Vec2> gridPos = GetCursorGridPos();
    if (gridPos)
    {
        UpdateMouseState(*gridPos);
        m_draw.DrawSelection({ 
            m_grid.GLCoords(gridPos->X, gridPos->Y), 
            m_grid.GLCellDimensions() 
        });
    }
}

void gol::Game::Begin()
{
    const std::chrono::steady_clock clock{};
    double lastTimeMs = GetTimeMs(clock);

    while (!glfwWindowShouldClose(m_window))
    {
        GL_DEBUG(glfwPollEvents());

        UpdateViewport();
        UpdateState();

        switch (m_settings.State)
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

        GL_DEBUG(glfwSwapBuffers(m_window));
    }
}