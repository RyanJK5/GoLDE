#include <fstream>
#include <chrono>
#include <iostream>

#include "vendor/imgui.h"
#include "vendor/imgui_impl_glfw.h"
#include "vendor/imgui_impl_opengl3.h"

#include "ShaderManager.h"
#include "GLException.h"
#include "Logging.h"
#include "GameWindow.h"

static void FrameResized(GLFWwindow* window, int width, int height)
{
    glViewport(0.25 * width, 0.25 * height, 0.75 * width, 0.75 * height);
}

std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> gol::GameWindow::ViewportBounds() const
{
    int32_t width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    return std::make_tuple(0.25 * width, 0, 0.75 * width, 0.75 * height);
}

gol::GameWindow::GameWindow()
{
    std::ifstream seed("seeds\\test.bin", std::ios::binary);
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(seed), {});
    
    m_grid = GameGrid();

    try
    {
        if (!glfwInit())
            throw GLException("Failed to initialize glfw");

        m_window = glfwCreateWindow(1920, 1080, "Conway's Game of Life", NULL, NULL);

        if (!m_window)
            throw GLException("Failed to create window");

        glfwMakeContextCurrent(m_window);

        glLineWidth(4);

        glfwSetWindowAspectRatio(m_window, 16, 9);
        glfwSwapInterval(1);
        FrameResized(m_window, 1920, 1080);

        glfwSetFramebufferSizeCallback(m_window, FrameResized);

        if (glewInit() != GLEW_OK)
            throw GLException("Failed to initialize glew");

        gol::ShaderManager shader("shader/default.shader");
        GL_DEBUG(glUseProgram(shader.Program()));
    }
    catch (GLException e)
    {
        glfwTerminate();
        throw e;
    }
}

gol::GameWindow::~GameWindow()
{
    glfwTerminate();
}

void gol::GameWindow::DrawGrid() const
{
    std::vector<float> positions = m_grid.GenerateGLBuffer();
    unsigned int buffer;
    
    GL_DEBUG(glGenBuffers(1, &buffer));
    GL_DEBUG(glBindBuffer(GL_ARRAY_BUFFER, buffer));
    GL_DEBUG(glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_DYNAMIC_DRAW));

    GL_DEBUG(glEnableVertexAttribArray(0));
    GL_DEBUG(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));

    GL_DEBUG(glDrawArrays(GL_QUADS, 0, positions.size() / 2));
}

void gol::GameWindow::DrawSelection(size_t x, size_t y) const
{
    auto [glX, glY] = m_grid.GLCoords(x, y);
    auto [glW, glH] = m_grid.GLCellDimensions();
    float_t positions[] = {
        glX,       glY,
        glX,       glY - glH,
        glX + glW, glY - glH,
        glX + glW, glY
    };
    uint32_t buffer;

    uint8_t indices[] = {
        0, 1,
        1, 2,
        2, 3,
        3, 0
    };
    uint32_t indexBuffer;

    GL_DEBUG(glGenBuffers(1, &buffer));
    GL_DEBUG(glBindBuffer(GL_ARRAY_BUFFER, buffer));
    GL_DEBUG(glBufferData(GL_ARRAY_BUFFER, 2 * 4 * sizeof(float_t), positions, GL_DYNAMIC_DRAW));
    GL_DEBUG(glEnableVertexAttribArray(0));
    GL_DEBUG(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float_t) * 2, 0));

    GL_DEBUG(glGenBuffers(1, &indexBuffer));
    GL_DEBUG(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer));
    GL_DEBUG(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * 4 * sizeof(uint8_t), indices, GL_DYNAMIC_DRAW));
    
    GL_DEBUG(glDrawElements(GL_LINES, 8, GL_UNSIGNED_BYTE, nullptr));
}

static float getTimeMs(const std::chrono::steady_clock& clock)
{
    return clock.now().time_since_epoch().count() / powf(10, 6);
}


void gol::GameWindow::GameLoop()
{
    double tickTime = 100;

    std::chrono::steady_clock clock;
    float last = getTimeMs(clock);

    bool enterDown = false;

    while (!glfwWindowShouldClose(m_window))
    {
        int enterState = glfwGetKey(m_window, GLFW_KEY_ENTER);
        if (enterState == GLFW_PRESS)
            enterDown = true;
        if (enterDown && enterState == GLFW_RELEASE)
            break;

        float now = getTimeMs(clock);
        if (now - last < tickTime)
            continue;
        last = now + (now - last - tickTime);

        if (m_grid.Dead())
            break;
        m_grid.Update();
        
        GL_DEBUG(glClear(GL_COLOR_BUFFER_BIT));
        DrawGrid();

        GL_DEBUG(glfwSwapBuffers(m_window));
        GL_DEBUG(glfwPollEvents());
    }

    if (!glfwWindowShouldClose(m_window))
    {
        m_grid = GameGrid();
        Begin();
    }
}

void gol::GameWindow::PaintUpdate()
{
    static std::optional<bool > drawMode = std::nullopt;

    double x, y;
    glfwGetCursorPos(m_window, &x, &y);

    auto [viewX, viewY, viewWidth, viewHeight] = ViewportBounds();

    if (x < viewX || y < viewY || x >= viewX + viewWidth || y >= viewY + viewHeight)
        return;


    size_t xPos = (x - viewX) / (float(viewWidth) / m_grid.Width());
    size_t yPos = (y - viewY) / (float(viewHeight) / m_grid.Height());

    if (xPos < 0 || xPos >= m_grid.Width() || yPos < 0 || yPos >= m_grid.Height())
        return;
    
    int mouseState = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT);
    if (mouseState == GLFW_PRESS)
    {
        if (!drawMode)
            drawMode = !m_grid.Get(xPos, yPos);

        m_grid.Set(xPos, yPos, drawMode.value());
    }
    else if (mouseState == GLFW_RELEASE)
        drawMode = std::nullopt;

    DrawSelection(xPos, yPos);
}

void gol::GameWindow::PaintLoop()
{
    bool enterDown = false;
    
    while (!glfwWindowShouldClose(m_window))
    {
        int enterState = glfwGetKey(m_window, GLFW_KEY_ENTER);
        if (enterState == GLFW_PRESS)
            enterDown = true;
        if (enterDown && enterState == GLFW_RELEASE)
            break;
        
        GL_DEBUG(glClear(GL_COLOR_BUFFER_BIT));
        DrawGrid();

        PaintUpdate();

        GL_DEBUG(glfwSwapBuffers(m_window));
        GL_DEBUG(glfwPollEvents());
    }

    if (!glfwWindowShouldClose(m_window))
        GameLoop();
}

void gol::GameWindow::Begin()
{
    PaintLoop();
}