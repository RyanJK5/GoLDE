#include <exception>

#include "Logging.h"
#include "SimulationEditor.h"

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
        if (m_Grid.Dead())
            return GameState::Empty;
    }
    m_Graphics.DrawGrid(m_Grid.Data(), args);
    return GameState::Simulation;
}

gol::GameState gol::SimulationEditor::PaintUpdate(const GraphicsHandlerArgs& args)
{
    m_Graphics.DrawGrid(m_Grid.Data(), args);

    auto gridPos = CursorGridPos();
    if (gridPos)
    {
        m_Graphics.DrawSelection(*gridPos, args);
        UpdateMouseState(*gridPos);
    }

    return m_Grid.Dead() 
        ? GameState::Empty 
        : GameState::Paint;
}

gol::GameState gol::SimulationEditor::PauseUpdate(const GraphicsHandlerArgs& args)
{
    m_Graphics.DrawGrid(m_Grid.Data(), args);
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
    ImGui::Text(std::format("Population: {}", m_Grid.Population()).c_str());

    auto cursorPos = CursorGridPos();
    if (cursorPos)
    {
        std::string text = std::format("({}, {})", cursorPos->X, cursorPos->Y);
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

gol::GameState gol::SimulationEditor::UpdateState(const SimulationControlResult& result)
{
    switch (result.Action)
    {
    using enum GameAction;
    case Start:
        m_InitialGrid = m_Grid;
        return GameState::Simulation;
    case Clear:
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
        return GameState::Simulation;
    case Step:
    {
        for (int32_t i = 0; i < *result.StepCount; i++)
            m_Grid.Update();
        return m_Grid.Dead() ? GameState::Empty : GameState::Paused;
    }
    case Resize:
    {
        m_Grid = GameGrid(m_Grid, *result.NewDimensions);
        m_Graphics.Camera.Center = { result.NewDimensions->Width * DefaultCellWidth / 2.f, result.NewDimensions->Height * DefaultCellHeight / 2.f };
        return GameState::Paint;
    }
    }
    throw std::exception("Cannot pass 'None' as action to UpdateState");
}

void gol::SimulationEditor::UpdateMouseState(Vec2 gridPos)
{
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        if (m_DrawMode == DrawMode::None)
            m_DrawMode = *m_Grid.Get(gridPos.X, gridPos.Y) ? DrawMode::Delete : DrawMode::Insert;

        m_Grid.Set(gridPos.X, gridPos.Y, m_DrawMode == DrawMode::Insert);
        return;
    }
    m_DrawMode = DrawMode::None;
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
    if (bounds.InBounds(mousePos.x, mousePos.y))
    {
        if (ImGui::GetIO().MouseWheel != 0)
        {
            m_Graphics.Camera.ZoomBy(mousePos, bounds, ImGui::GetIO().MouseWheel / 10.f);
        }
    }

    glViewport(static_cast<int32_t>(bounds.X - m_WindowBounds.X), static_cast<int32_t>(bounds.Y - m_WindowBounds.Y), bounds.Width, bounds.Height);
}