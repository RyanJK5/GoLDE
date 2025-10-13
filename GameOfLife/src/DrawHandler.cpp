#include "DrawHandler.h"
#include "Logging.h"

#include "vendor/imgui.h"
#include "vendor/imgui_impl_glfw.h"
#include "vendor/imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

void gol::DrawHandler::ClearBackground(const Rect& windowBounds, const Rect& viewportBounds) const {
    GL_DEBUG(glEnable(GL_SCISSOR_TEST));
    
    GL_DEBUG(glScissor(0, 0, windowBounds.Width, windowBounds.Height));
    GL_DEBUG(glClearColor(0.1f, 0.1f, 0.1f, 1));
    GL_DEBUG(glClear(GL_COLOR_BUFFER_BIT));
    
    GL_DEBUG(glScissor(
        viewportBounds.X, viewportBounds.Y, 
        viewportBounds.Width, viewportBounds.Height
    ));
    GL_DEBUG(glClearColor(0, 0, 0, 1));
    GL_DEBUG(glClear(GL_COLOR_BUFFER_BIT));

    GL_DEBUG(glDisable(GL_SCISSOR_TEST));
}

void gol::DrawHandler::DrawGrid(const std::vector<float>& positions) const
{
    uint32_t buffer;
    GL_DEBUG(glGenBuffers(1, &buffer));
    GL_DEBUG(glBindBuffer(GL_ARRAY_BUFFER, buffer));
    GL_DEBUG(glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_DYNAMIC_DRAW));

    GL_DEBUG(glEnableVertexAttribArray(0));
    GL_DEBUG(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));

    GL_DEBUG(glDrawArrays(GL_QUADS, 0, positions.size() / 2));
    GL_DEBUG(glDeleteBuffers(1, &buffer));
}

void gol::DrawHandler::DrawSelection(const RectF& bounds) const
{
    float positions[] = 
    {
        bounds.X,       bounds.Y,
        bounds.X,       bounds.Y - bounds.Height,
        bounds.X + bounds.Width, bounds.Y - bounds.Height,
        bounds.X + bounds.Width, bounds.Y
    };
    uint32_t buffer;

    uint8_t indices[] = 
    {
        0, 1,
        1, 2,
        2, 3,
        3, 0
    };
    uint32_t indexBuffer;

    GL_DEBUG(glGenBuffers(1, &buffer));
    GL_DEBUG(glBindBuffer(GL_ARRAY_BUFFER, buffer));
    GL_DEBUG(glBufferData(GL_ARRAY_BUFFER, 2 * 4 * sizeof(float), positions, GL_DYNAMIC_DRAW));
    GL_DEBUG(glEnableVertexAttribArray(0));
    GL_DEBUG(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));

    GL_DEBUG(glGenBuffers(1, &indexBuffer));
    GL_DEBUG(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer));
    GL_DEBUG(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * 4 * sizeof(uint8_t), indices, GL_DYNAMIC_DRAW));

    GL_DEBUG(glDrawElements(GL_LINES, 8, GL_UNSIGNED_BYTE, nullptr));
    GL_DEBUG(glDeleteBuffers(1, &buffer));
}