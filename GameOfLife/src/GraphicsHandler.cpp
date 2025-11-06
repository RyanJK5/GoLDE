#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "SimulationEditor.h"
#include "GraphicsHandler.h"
#include "Logging.h"
#include "GLException.h"

gol::GraphicsHandler::GraphicsHandler(const std::filesystem::path& shaderFilePath, int32_t windowWidth, int32_t windowHeight)
    : m_Shader(shaderFilePath)
{
    GL_DEBUG(glGenBuffers(1, &m_GridBuffer));
    GL_DEBUG(glGenBuffers(1, &m_SelectionBuffer));
    GL_DEBUG(glGenBuffers(1, &m_SelectionIndexBuffer));

    GL_DEBUG(glGenFramebuffers(1, &m_FrameBuffer));
    GL_DEBUG(glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer));

    GL_DEBUG(glGenTextures(1, &m_Texture));
    GL_DEBUG(glBindTexture(GL_TEXTURE_2D, m_Texture));
    GL_DEBUG(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL));
    GL_DEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_DEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_DEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture, 0));

    GL_DEBUG(glGenRenderbuffers(1, &m_renderBuffer));
    GL_DEBUG(glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer));
    GL_DEBUG(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight));
    GL_DEBUG(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderBuffer));

    GL_DEBUG(
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            throw GLException("Framebuffer not properly initialized");
    );
    GL_DEBUG(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    GL_DEBUG(glBindTexture(GL_TEXTURE_2D, 0));
    GL_DEBUG(glBindRenderbuffer(GL_RENDERBUFFER, 0));

    GL_DEBUG(glUseProgram(m_Shader.Program()));
}

gol::GraphicsHandler::GraphicsHandler(GraphicsHandler&& other) noexcept
    : m_Shader(std::forward<ShaderManager>(other.m_Shader))
{
    Move(std::forward<GraphicsHandler>(other));
}

gol::GraphicsHandler& gol::GraphicsHandler::operator=(GraphicsHandler&& other) noexcept
{
    if (this != &other)
    {
        m_Shader = std::move(other.m_Shader);
        Move(std::forward<GraphicsHandler>(other));
        Destroy();
    }
    return *this;
}

gol::GraphicsHandler::~GraphicsHandler()
{
    Destroy();
}

void gol::GraphicsHandler::Move(GraphicsHandler&& other) noexcept
{
    m_GridBuffer = std::exchange(other.m_GridBuffer, 0);
    m_SelectionBuffer = std::exchange(other.m_SelectionBuffer, 0);
    m_SelectionIndexBuffer = std::exchange(other.m_SelectionIndexBuffer, 0);
    m_FrameBuffer = std::exchange(other.m_FrameBuffer, 0);
    m_Texture = std::exchange(other.m_Texture, 0);
    m_renderBuffer = std::exchange(other.m_renderBuffer, 0);
}

void gol::GraphicsHandler::Destroy()
{
    GL_DEBUG(glDeleteBuffers(1, &m_SelectionBuffer));
    GL_DEBUG(glDeleteFramebuffers(1, &m_FrameBuffer));
    GL_DEBUG(glDeleteTextures(1, &m_Texture));
    GL_DEBUG(glDeleteRenderbuffers(1, &m_renderBuffer));
}

void gol::GraphicsHandler::RescaleFrameBuffer(Size2 windowSize)
{
    GL_DEBUG(glBindTexture(GL_TEXTURE_2D, m_Texture));
    GL_DEBUG(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowSize.Width, windowSize.Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));
    GL_DEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_DEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_DEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture, 0));
    GL_DEBUG(glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer));
    GL_DEBUG(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowSize.Width, windowSize.Height));
    GL_DEBUG(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderBuffer));
}

void gol::GraphicsHandler::BindFrameBuffer() const
{
    GL_DEBUG(glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer));
}

void gol::GraphicsHandler::UnbindFrameBuffer() const
{
    GL_DEBUG(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void gol::GraphicsHandler::ClearBackground(const GraphicsHandlerArgs& args) const
{
    BindFrameBuffer();

    if (args.GridSize.Width == 0 || args.GridSize.Height == 0)
    {
        GL_DEBUG(glClearColor(0.f, 0.f, 0.f, 1.f));
        GL_DEBUG(glClear(GL_COLOR_BUFFER_BIT));
    }
    else
    {
        GL_DEBUG(glEnable(GL_SCISSOR_TEST));
    
        GL_DEBUG(glScissor(0, 0, args.ViewportBounds.Width, args.ViewportBounds.Height));
        GL_DEBUG(glClearColor(0.1f, 0.1f, 0.1f, 1));
        GL_DEBUG(glClear(GL_COLOR_BUFFER_BIT));
    
        Size2F gridScreenDimensions = 
        { 
            args.GridSize.Width * SimulationEditor::DefaultCellWidth, 
            args.GridSize.Height * SimulationEditor::DefaultCellHeight
        };
        auto origin = Camera.WorldToScreenPos({ 0, 0 }, args.ViewportBounds, gridScreenDimensions);
        auto lowerRight = Camera.WorldToScreenPos({ gridScreenDimensions.Width, gridScreenDimensions.Height }, args.ViewportBounds, gridScreenDimensions);
        GL_DEBUG(glScissor(
            origin.x, origin.y,
            lowerRight.x - origin.x, lowerRight.y - origin.y
        ));
        GL_DEBUG(glClearColor(0.f, 0.f, 0.f, 1.f));
        GL_DEBUG(glClear(GL_COLOR_BUFFER_BIT));

        GL_DEBUG(glDisable(GL_SCISSOR_TEST));
    }

    UnbindFrameBuffer();
}

std::vector<float> gol::GraphicsHandler::GenerateGLBuffer(const std::set<Vec2>& grid) const
{
    float width = SimulationEditor::DefaultCellWidth;
    float height = SimulationEditor::DefaultCellHeight;
    std::vector<float> result;
    for (const Vec2& vec : grid)
    {
        float xCoord = vec.X * width;
        float yCoord = vec.Y * height;
        
        result.push_back(xCoord);
        result.push_back(yCoord);

        result.push_back(xCoord);
        result.push_back(yCoord + height);
            
        result.push_back(xCoord + width);
        result.push_back(yCoord + height);

        result.push_back(xCoord + width);
        result.push_back(yCoord);
    }

    return result;
}

void gol::GraphicsHandler::DrawGrid(const std::set<Vec2>& grid, const GraphicsHandlerArgs& args)
{
    BindFrameBuffer();
    auto matrix = Camera.OrthographicProjection(args.ViewportBounds.Size());
    m_Shader.AttachUniformMatrix4("u_MVP", matrix);

    auto positions = GenerateGLBuffer(grid);

    GL_DEBUG(glBindBuffer(GL_ARRAY_BUFFER, m_GridBuffer));
    GL_DEBUG(glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_DYNAMIC_DRAW));

    GL_DEBUG(glEnableVertexAttribArray(0));
    GL_DEBUG(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));

    GL_DEBUG(glDrawArrays(GL_QUADS, 0, positions.size() / 2));
    GL_DEBUG(glDeleteBuffers(1, &m_GridBuffer));
    
    UnbindFrameBuffer();
}

gol::RectF gol::GraphicsHandler::GridToScreenBounds(Vec2 gridPos, const GraphicsHandlerArgs&) const
{
    return {
          static_cast<float>(gridPos.X * SimulationEditor::DefaultCellHeight),
          static_cast<float>(gridPos.Y * SimulationEditor::DefaultCellHeight),
          SimulationEditor::DefaultCellWidth,
          SimulationEditor::DefaultCellHeight
    };
}

void gol::GraphicsHandler::DrawSelection(Vec2 gridPos, const GraphicsHandlerArgs& args)
{
    BindFrameBuffer();

    RectF rect = GridToScreenBounds(gridPos, args);
    float positions[] = 
    { 
        rect.UpperLeft().X, rect.UpperLeft().Y,
        rect.LowerLeft().X, rect.LowerLeft().Y,
        rect.LowerRight().X, rect.LowerRight().Y,
        rect.UpperRight().X, rect.UpperRight().Y 
    };

    uint8_t indices[] = 
    {
        0, 1,
        1, 2,
        2, 3,
        3, 0
    };

    GL_DEBUG(glBindBuffer(GL_ARRAY_BUFFER, m_SelectionBuffer));
    GL_DEBUG(glBufferData(GL_ARRAY_BUFFER, 2 * 4 * sizeof(float), positions, GL_DYNAMIC_DRAW));
    GL_DEBUG(glEnableVertexAttribArray(0));
    GL_DEBUG(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));

    GL_DEBUG(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_SelectionIndexBuffer));
    GL_DEBUG(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * 4 * sizeof(uint8_t), indices, GL_DYNAMIC_DRAW));

    GL_DEBUG(glDrawElements(GL_LINES, 8, GL_UNSIGNED_BYTE, nullptr));

    UnbindFrameBuffer();
}