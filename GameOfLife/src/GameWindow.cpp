#include "Logging.h"
#include "GameWindow.h"
#include "GLException.h"

#include "vendor/imgui.h"
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
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= GOLImGuiConfig;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    ImGui_ImplGlfw_InitForOpenGL(m_Window.get(), true);
    ImGui_ImplOpenGL3_Init();
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
        return Rect{ window.X + newX, window.Y, newWidth, window.Height};
    }
    const int32_t newHeight = static_cast<int32_t>(widthRatio * gridSize.Height);
    const int32_t newY = (window.Height - newHeight) / 2;
    return Rect{ window.X, window.Y + newY, window.Width, newHeight };
}

gol::Vec2F gol::GameWindow::CursorPos() const
{
    return ImGui::GetMousePos();
}

void gol::GameWindow::FrameStart(uint32_t textureID)
{
    GL_DEBUG(glfwPollEvents());

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport();

    GL_DEBUG(glClear(GL_COLOR_BUFFER_BIT));
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

void gol::GameWindow::FrameEnd() const
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    GLFWwindow* backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);

    GL_DEBUG(glfwSwapBuffers(m_Window.get()));
}

void gol::GameWindow::UpdateViewport(Size2 gridSize) const
{
    Rect bounds = ViewportBounds(gridSize);
    glViewport(bounds.X - m_WindowBounds.X, bounds.Y - m_WindowBounds.Y, bounds.Width, bounds.Height);
}