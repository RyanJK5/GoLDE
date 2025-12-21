#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <format>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <optional>
#include <utility>

#include "Game.h"
#include "GLException.h"
#include "ConfigLoader.h"
#include "Logging.h"
#include "GameEnums.h"
#include "Graphics2D.h"
#include "SimulationEditor.h"
#include "PresetSelectionResult.h"
#include "SimulationControlResult.h"
 
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
    , m_Editors()
    , m_Control(*(StyleLoader::LoadYAML<ImVec4>(std::filesystem::path("config") / "style.yaml")))
    , m_PresetSelection(std::filesystem::current_path() / "templates")
{
    m_Editors.emplace_back(
        0u,
        std::filesystem::path { },
        Size2 { DefaultWindowWidth, DefaultWindowHeight },
        Size2 { DefaultGridWidth, DefaultGridHeight }
    );
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
        
        auto controlResult = m_Control.Update(m_State);
        auto presetResult = m_PresetSelection.Update();
        UpdateEditors(controlResult, presetResult);

        EndFrame();
    }
}

void gol::Game::UpdateEditors(const SimulationControlResult& controlResult, const PresetSelectionResult& presetResult)
{
    ImGui::Begin("###EditorDockspace", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration);
    
    ImGuiID subDockspace = ImGui::GetID("###EditorDockspace");
    ImGui::DockSpace(subDockspace, ImGui::GetContentRegionAvail(), ImGuiDockNodeFlags_None);
    if (m_Startup)
		CreateEditorDockspace();

    auto newWindowIndex = m_LastActive;
    bool pastWindowEnabled = CheckForNewEditors(controlResult);


    if (m_LastActive < m_Editors.size())
        std::swap(m_Editors[m_LastActive], m_Editors.back());
    m_LastActive = m_Editors.size() - 1;
    for (size_t i = 0; i < m_Editors.size(); i++)
    {
        auto activeOverride = [&]() -> std::optional<bool>
        {
            if (newWindowIndex != i && !pastWindowEnabled)
                return false;
            if (m_LastActive == i)
                return true;
            return std::nullopt;
        }();

        auto result = m_Editors[i].Update(activeOverride, controlResult, presetResult);

        if (result.Active)
        {
            m_State = result;
            m_LastActive = i;
        }
        if (result.Closing)
            m_Editors.erase(m_Editors.begin() + i--);
    }

    ImGui::End();
}

void gol::Game::InitImGUI(const std::filesystem::path& stylePath)
{
    auto styleInfo = StyleLoader::LoadYAML<ImVec4>(stylePath);
    if (!styleInfo)
        throw StyleLoader::StyleLoaderException(styleInfo.error());

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigDebugHighlightIdConflicts = true;

    io.IniFilename = nullptr;

    auto path = std::filesystem::path("resources") / "font" / "arial.ttf";
    m_Font = io.Fonts->AddFontFromFileTTF(path.string().c_str(), 30.0f);

	auto iconPath = std::filesystem::path("resources") / "font" / "fontawesome7.otf";
    auto config = ImFontConfig {};
    config.MergeMode = true;
    io.Fonts->AddFontFromFileTTF(iconPath.string().c_str(), 30.0f, &config);

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
	m_Startup = false;
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

    ImGuiID dockspaceID = ImGui::GetID("DockSpace");
    ImGui::DockSpaceOverViewport(dockspaceID, ImGui::GetMainViewport());
    if (m_Startup)
        InitDockspace(dockspaceID, windowSize);
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
    ImGui::DockBuilderDockWindow("###EditorDockspace", rightID);
    ImGui::DockBuilderDockWindow("Simulation Control", leftID);
    ImGui::DockBuilderFinish(dockspaceID);
}

bool gol::Game::CheckForNewEditors(const SimulationControlResult& controlResult)
{
    if (!controlResult.FilePath)
        return true;
    if (!controlResult.Action)
		return true;
	if (controlResult.Action != ActionVariant{ EditorAction::Load })
        return true;

    const bool fileOpen = std::ranges::any_of(m_Editors, [path = controlResult.FilePath](const SimulationEditor& editor) 
    { 
        return editor.CurrentFilePath() == path; 
    });
    if (fileOpen)
        return false;

    m_Editors.emplace_back(
        static_cast<uint32_t>(m_Editors.size()),
        *controlResult.FilePath,
        Size2{ DefaultWindowWidth, DefaultWindowHeight },
        Size2{ DefaultGridWidth, DefaultGridHeight }
    );

    CreateEditorDockspace();

    return false;
}

void gol::Game::CreateEditorDockspace()
{
	ImGuiID editorDockspaceID = ImGui::GetID("###EditorDockspace");
    auto id = ImGui::DockBuilderAddNode(
        editorDockspaceID,
        ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace
    );

    for (size_t i = 0; i < m_Editors.size(); i++)
    {
        ImGui::DockBuilderDockWindow(
            std::format("###Simulation{}", i).c_str(),
            editorDockspaceID
        );
    }
    ImGui::DockBuilderFinish(editorDockspaceID);
}
