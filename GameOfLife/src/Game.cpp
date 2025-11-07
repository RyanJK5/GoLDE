#include <fstream>
#include <iostream>
#include <algorithm>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "ShaderManager.h"
#include "GLException.h"
#include "Logging.h"
#include "GUILoader.h"
#include "Game.h"
#include "Graphics2D.h"

gol::OpenGLWindow::OpenGLWindow(int32_t width, int32_t height)
    : Bounds(0, 0, width, height)
{
    if (!glfwInit())
        throw GLException("Failed to initialize glfw");    

    Underlying = glfwCreateWindow(width, height, "Conway's Game of Life", NULL, NULL);

    if (!Underlying)
        throw GLException("Failed to create window");

    glfwMakeContextCurrent(Underlying);
    GL_DEBUG(glLineWidth(4));
    GL_DEBUG(glfwSwapInterval(1));

    if (glewInit() != GLEW_OK)
        throw GLException("Failed to initialize glew");
}

gol::OpenGLWindow::~OpenGLWindow() 
{
    glfwTerminate();
}

gol::Game::Game()
    : m_Window(DefaultWindowWidth, DefaultWindowHeight)
    , m_Editor({DefaultWindowWidth, DefaultWindowHeight}, {DefaultGridWidth, DefaultGridHeight})
    , m_Control(*(StyleLoader::LoadYAML<ImVec4>(std::filesystem::path("config") / "style.yaml")))
{
    InitImGUI(std::filesystem::path("config") / "style.yaml");
}

gol::Game::~Game()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void gol::Game::Begin()
{
    while (Open())
    {
        BeginFrame();
        
        auto result = m_Control.Update(m_State);
        m_State = m_Editor.Update(result);
        
        ImGui::Begin("Presets");
        ImGui::End();
        
        EndFrame();
    }
}

void gol::Game::InitImGUI(const std::filesystem::path& stylePath)
{
    auto styleInfo = StyleLoader::LoadYAML<ImVec4>(stylePath);
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
        style.Colors[pair.first] = styleInfo->StyleColors[pair.second];
    
    ImGui_ImplGlfw_InitForOpenGL(m_Window.Get(), true);
    ImGui_ImplOpenGL3_Init();
}

void gol::Game::BeginFrame()
{
    GL_DEBUG(glfwPollEvents());
    GL_DEBUG(glClear(GL_COLOR_BUFFER_BIT));

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::PushFont(m_Font, 30.0f);
    CreateDockspace();
}

void gol::Game::EndFrame()
{
    ImGui::PopFont();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    GLFWwindow* backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);

    GL_DEBUG(glfwSwapBuffers(m_Window.Get()));
}

void gol::Game::CreateDockspace()
{
    ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Once);

    int32_t windowWidth, windowHeight;
    GL_DEBUG(glfwGetFramebufferSize(m_Window.Get(), &windowWidth, &windowHeight));
    const ImVec2 windowSize = { static_cast<float>(windowWidth), static_cast<float>(windowHeight) };
    ImGui::SetNextWindowSize(windowSize);

    ImGui::Begin("DockSpace", nullptr, DockspaceFlags | ImGuiWindowFlags_NoTitleBar);
    ImGuiID dockspaceID = ImGui::GetID("DockSpace");
    ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    if (m_Startup)
    {
        InitDockspace(dockspaceID, windowSize);
        m_Startup = false;
    }

    ImGui::End();
}

void gol::Game::InitDockspace(uint32_t dockspaceID, ImVec2 windowSize)
{
    ImGui::DockBuilderRemoveNode(dockspaceID);
    ImGui::DockBuilderAddNode(
        dockspaceID,
        
        ImGuiDockNodeFlags_PassthruCentralNode | static_cast<ImGuiDockNodeFlags>(ImGuiDockNodeFlags_DockSpace)
    );
    ImGui::DockBuilderSetNodeSize(dockspaceID, windowSize);

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