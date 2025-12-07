#include <algorithm>
#include <format>
#include <imgui/imgui.h>
#include <filesystem>
#include <utility>

#include "FileDialog.h"
#include "Graphics2D.h"
#include "GraphicsHandler.h"
#include "Logging.h"
#include "PresetSelection.h"
#include "PresetSelectionResult.h"
#include "RLEEncoder.h"

gol::PresetSelection::PresetSelection(const std::filesystem::path& defaultPath, Size2 windowSize)
{
	ReadFiles(defaultPath, windowSize);
}

gol::PresetSelectionResult gol::PresetSelection::Update()
{
    ImGui::Begin("Presets", nullptr, ImGuiWindowFlags_NoNavInputs);
    //ImGui::BeginChild("Render", { 0, 0 }, ImGuiChildFlags_None, ImGuiWindowFlags_NoNavInputs);

    auto cursorPos = ImGui::GetCursorPos();
    for (size_t i = 0; i < m_Library.size(); i++)
    {
        auto windowBounds = RectF
        { 
            Vec2F { cursorPos },
            Size2F { static_cast<float>(TemplateDimensions.Width), static_cast<float>(TemplateDimensions.Height) }
        };
        auto cellSize = std::min(windowBounds.Width / m_MaxGridDimensions.Width, windowBounds.Height / m_MaxGridDimensions.Height);

        auto graphicsArgs = GraphicsHandlerArgs
        {
            .ViewportBounds = windowBounds,
            .GridSize = m_Library[i].Grid.Size(),
            .CellSize = {cellSize, cellSize},
            .ShowGridLines = true
        };

        m_Library[i].Graphics.RescaleFrameBuffer(windowBounds, windowBounds);
        m_Library[i].Graphics.CenterCamera(graphicsArgs);
        m_Library[i].Graphics.ClearBackground(graphicsArgs);

        m_Library[i].Graphics.DrawGrid(
            { 0, 0 },
            m_Library[i].Grid.Data(),
            graphicsArgs
        );

        if (i % TemplatesPerRow != 0)
            ImGui::SameLine();
        cursorPos = ImGui::GetCursorPos();

        ImGui::Image(
            static_cast<ImTextureID>(m_Library[i].Graphics.TextureID()),
            { windowBounds.Width, windowBounds.Height},
            ImVec2(0, 1),
            ImVec2(1, 0)
        );
        ImGui::SetItemTooltip(m_Library[i].FileName.c_str());

        ImGui::SetCursorPos(cursorPos);
        if (ImGui::InvisibleButton(std::format("##{}", m_Library[i].FileName).c_str(), {windowBounds.Width, windowBounds.Height}))
        {
            ImGui::SetClipboardText(RLEEncoder::EncodeRegion(m_Library[i].Grid, {{0, 0}, m_Library[i].Grid.Size()}).c_str());
        }
    }

    //ImGui::EndChild();
    ImGui::End();

    return {};
}

void gol::PresetSelection::ReadFiles(const std::filesystem::path& defaultPath, Size2 windowSize)
{
    m_MaxGridDimensions = Size2F {};
    for (const auto& file : std::filesystem::directory_iterator(defaultPath)) 
    {
        if (file.path().extension() != ".gol")
            continue;

        auto result = RLEEncoder::ReadRegion(file.path());
        if (!result)
        {
			ERROR("Failed to open file {}: {}", file.path().filename().string(), result.error());
            continue;
        }
        
        m_MaxGridDimensions.Width = std::max(m_MaxGridDimensions.Width, static_cast<float>(result->Grid.Width()));
        m_MaxGridDimensions.Height = std::max(m_MaxGridDimensions.Height, static_cast<float>(result->Grid.Height()));

        m_Library.emplace_back(
            std::move(result->Grid),
            file.path().filename().string(),
            Size2 { windowSize.Width, windowSize.Height }
        );
    }
}