#include <string_view>

#include "Logging.h"
#include "GameWindow.h"
#include "GUILoader.h"
#include "GLException.h"

#include "vendor/imgui.h"
#include "vendor/imgui_internal.h"
#include "vendor/imgui_impl_glfw.h"
#include "vendor/imgui_impl_opengl3.h"

gol::GameWindow::GameWindow(Size2 size)
    : GameWindow(size.Width, size.Height)
{ }

gol::GameWindow::GameWindow(int32_t width, int32_t height)
    : m_WindowBounds(0, 0, width, height)
{
    if (!glfwInit())
        throw GLException("Failed to initialize glfw");

    m_Window = std::unique_ptr<GLFWwindow>(
        glfwCreateWindow(width, height, "Conway's Game of Life", NULL, NULL)
    );

    if (!m_Window)
        throw GLException("Failed to create window");

    glfwMakeContextCurrent(m_Window.get());
    GL_DEBUG(glLineWidth(4));
    GL_DEBUG(glfwSwapInterval(1));

    if (glewInit() != GLEW_OK)
        throw GLException("Failed to initialize glew");

    InitImGUI();
}

gol::GameWindow::~GameWindow()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void gol::GameWindow::InitImGUI()
{
    auto styleInfo = StyleLoader::LoadStyle<ImVec4>(std::filesystem::path("config") / "style.yaml");
    if (!styleInfo)
        throw StyleLoader::StyleLoaderException(styleInfo.error());

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

     ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= IOFlags;
    io.ConfigDebugHighlightIdConflicts = true;

    auto path = std::filesystem::path("resources") / "font" / "arial.ttf";
    m_Font = io.Fonts->AddFontFromFileTTF(path.string().c_str(), 30.0f);

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowMenuButtonPosition = ImGuiDir_None;

    for (auto&& pair : styleInfo->AttributeColors)
    {
        style.Colors[pair.first] = styleInfo->StyleColors[pair.second];
    }

    ImGui_ImplGlfw_InitForOpenGL(m_Window.get(), true);
    ImGui_ImplOpenGL3_Init();
    
    CreateButtons(styleInfo->Shortcuts);
}

void gol::GameWindow::CreateButtons(const std::unordered_map<GameAction, std::vector<ImGuiKeyChord>>& shortcuts)
{
    Size2F topRow = { 100, 50 };
    Size2F bottomRow = { 155, 50 };
    m_Buttons.emplace_back(
        "Start",
        GameAction::Start,
        topRow,
        [](auto info) { return !info.GridDead && (info.State == GameState::Paint || info.State == GameState::Paused); },
        shortcuts.at(GameAction::Start),
        true
    );
    m_Buttons.emplace_back(
        "Clear",
        GameAction::Clear,
        topRow,
        [](auto info) { return !info.GridDead; },
        shortcuts.at(GameAction::Clear)
    );
    m_Buttons.emplace_back(
        "Reset",
        GameAction::Reset,
        topRow,
        [](auto info) { return info.State == GameState::Simulation; },
        shortcuts.at(GameAction::Reset)
    );
    m_Buttons.emplace_back(
        "Pause",
        GameAction::Pause,
        bottomRow,
        [](auto info) { return info.State == GameState::Simulation; },
        shortcuts.at(GameAction::Pause),
        true
    );
    m_Buttons.emplace_back(
        "Resume",
        GameAction::Resume,
        bottomRow,
        [](auto info) { return info.State == GameState::Paused; },
        shortcuts.at(GameAction::Resume)
    );
}

gol::Rect gol::GameWindow::WindowBounds() const
{
    return Rect(
        static_cast<int32_t>(m_WindowBounds.X), 
        static_cast<int32_t>(m_WindowBounds.Y), 
        static_cast<int32_t>(m_WindowBounds.Width),
        static_cast<int32_t>(m_WindowBounds.Height)
    );
}

gol::Rect gol::GameWindow::ViewportBounds(Size2 gridSize) const
{
    Rect window = WindowBounds();
    const float widthRatio = static_cast<float>(window.Width) / gridSize.Width;
    const float heightRatio = static_cast<float>(window.Height) / gridSize.Height;
    if (widthRatio > heightRatio)
    {
        const int32_t newWidth = static_cast<int32_t>(heightRatio * gridSize.Width);
        const int32_t newX = (window.Width - newWidth) / 2;
        return Rect { window.X + newX, window.Y, newWidth, window.Height};
    }
    const int32_t newHeight = static_cast<int32_t>(widthRatio * gridSize.Height);
    const int32_t newY = (window.Height - newHeight) / 2;
    return Rect { window.X, window.Y + newY, window.Width, newHeight };
}

gol::Vec2F gol::GameWindow::CursorPos() const
{
    return ImGui::GetMousePos();
}

void gol::GameWindow::BeginFrame()
{
    GL_DEBUG(glfwPollEvents());
    GL_DEBUG(glClear(GL_COLOR_BUFFER_BIT));

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void gol::GameWindow::DisplaySimulation(uint32_t textureID)
{
    ImGui::Begin("Simulation");
    {
        ImGui::BeginChild("GameRender");

        m_WindowBounds = { Vec2F(ImGui::GetWindowPos()), Size2F(ImGui::GetContentRegionAvail()) };

        ImGui::Image(
            (ImTextureID)textureID,
            ImGui::GetContentRegionAvail(),
            ImVec2(0, 1),
            ImVec2(1, 0)
        );
    }
    ImGui::EndChild();
    ImGui::End();
}

gol::GameAction gol::GameWindow::DisplaySimulationControl(const DrawInfo& info)
{
    ImGui::Begin("Simulation Control");

    GameAction result = GameAction::None;
    for (auto& button : m_Buttons)
    {
        GameAction action = button.Update(info);
        if (result == GameAction::None)
            result = action;
    }

    ImGui::End();
    return result;
}

gol::UpdateInfo gol::GameWindow::CreateGUI(const DrawInfo& info)
{
    UpdateInfo result = {};

    ImGui::PushFont(m_Font, 30.0f);
    CreateDockspace();

    result.Action = DisplaySimulationControl(info);
    DisplaySimulation(info.SimulationTextureID);
    

    ImGui::Begin("Presets");
    ImGui::Text("Hello, down!");
    ImGui::End();

    ImGui::PopFont();

    return result;
}

void gol::GameWindow::CreateDockspace()
{
    ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Once);

    int32_t windowWidth{}, windowHeight{};
    glfwGetFramebufferSize(m_Window.get(), &windowWidth, &windowHeight);
    ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));

    ImGui::Begin("DockSpace", nullptr, DockspaceFlags | ImGuiWindowFlags_NoTitleBar);
    ImGuiID dockspaceID = ImGui::GetID("DockSpace");
    ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    if (m_Startup)
    {
        InitDockspace(dockspaceID);
        m_Startup = false;
    }

    ImGui::End();
}

void gol::GameWindow::InitDockspace(uint32_t dockspaceID)
{
    ImGui::DockBuilderRemoveNode(dockspaceID);
    ImGui::DockBuilderAddNode(
        dockspaceID,
        ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace
    );
    ImGui::DockBuilderSetNodeSize(dockspaceID, { m_WindowBounds.Width, m_WindowBounds.Height });

    ImGuiID leftID{}, rightID{};
    ImGui::DockBuilderSplitNode(
        dockspaceID,
        ImGuiDir_Left,
        0.25f,
        &leftID,
        &rightID
    );
    auto downID = ImGui::DockBuilderSplitNode(
        rightID,
        ImGuiDir_Down,
        0.25f,
        nullptr,
        &rightID
    );

    ImGui::DockBuilderDockWindow("Presets", downID);
    ImGui::DockBuilderDockWindow("Simulation", rightID);
    ImGui::DockBuilderDockWindow("Simulation Control", leftID);
    ImGui::DockBuilderFinish(dockspaceID);
}

void gol::GameWindow::EndFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    GLFWwindow* backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);

    GL_DEBUG(glfwSwapBuffers(m_Window.get()));
}

void gol::GameWindow::UpdateViewport(Size2 gridSize)
{
    Rect bounds = ViewportBounds(gridSize);
    glViewport(bounds.X - m_WindowBounds.X, bounds.Y - m_WindowBounds.Y, bounds.Width, bounds.Height);
}