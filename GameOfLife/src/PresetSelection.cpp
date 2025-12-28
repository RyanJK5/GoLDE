#include <algorithm>
#include <imgui/imgui.h>
#include <filesystem>
#include <font-awesome/IconsFontAwesome7.h>
#include <format>
#include <string>
#include <utility>

#include "FileDialog.h"
#include "GameGrid.h"
#include "Graphics2D.h"
#include "GraphicsHandler.h"
#include "Logging.h"
#include "PresetSelection.h"
#include "PresetSelectionResult.h"
#include "RLEEncoder.h"

gol::SearchString::SearchString(size_t length)
    : Length(length), Data(new char[length + 1])
{
    for (size_t i = 0; i <= length; i++)
    	Data[i] = '\0';
}

gol::SearchString::SearchString(const SearchString& other)
{
    Copy(other);
}

gol::SearchString::SearchString(SearchString&& other) noexcept
{
    Move(std::move(other));
}

gol::SearchString& gol::SearchString::operator=(const SearchString& other)
{
    if (this != &other)
    {
        Destroy();
        Copy(other);
    }
    return *this;
}

gol::SearchString& gol::SearchString::operator=(SearchString&& other) noexcept
{
	if (this != &other)
    {
        Destroy();
        Move(std::move(other));
    }
    return *this;
}

gol::SearchString::~SearchString()
{
    Destroy();
}

void gol::SearchString::Copy(const SearchString& other)
{
    Length = other.Length;
    Data = new char[other.Length + 1];
    for (size_t i = 0; i <= other.Length + 1; i++)
        Data[i] = other.Data[i];
}

void gol::SearchString::Move(SearchString&& other)
{
    Length = std::exchange(other.Length, 0);
    Data = std::exchange(other.Data, nullptr);
}

void gol::SearchString::Destroy()
{
    delete[] Data;
}

gol::PresetDisplay::PresetDisplay(const GameGrid& grid, const std::string& fileName, Size2 windowSize)
    : Grid(grid)
    , FileName(fileName)
    , Graphics(
        std::filesystem::path("resources") / "shader",
        windowSize.Width, windowSize.Height,
        Color { }
    )
{ }

gol::PresetSelection::PresetSelection(const std::filesystem::path& defaultPath, Size2 windowSize)
	: m_DefaultPath(defaultPath)
    , m_WindowSize(windowSize)
{
	ReadFiles(m_DefaultPath);
	m_SearchText = SearchString(m_MaxFileName);
}

gol::PresetSelectionResult gol::PresetSelection::Update()
{
    ImGui::Begin("Presets", nullptr, ImGuiWindowFlags_NoNav);

    ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 10.f);

    ImGui::SetNextItemWidth(m_MaxFileName * 20.f);
    ImGui::InputTextWithHint("##search", "Search...", m_SearchText.Data, m_SearchText.Length + 1);

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FOLDER_OPEN))
    {
        auto result = FileDialog::SelectFolderDialog(m_DefaultPath.string());
        if (result)
        {
            m_Library.clear();
            ReadFiles(*result);
        }
    }

    ImGui::PopStyleVar();

    auto retString = std::string {};

    auto cursorPos = ImGui::GetCursorPos();
    size_t numAvailable = 0;
    for (size_t i = 0; i < m_Library.size(); i++)
    {
		if (!m_Library[i].FileName.contains(m_SearchText.Data))
            continue;

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
            .ShowGridLines = false
        };

        m_Library[i].Graphics.RescaleFrameBuffer(windowBounds, windowBounds);
        m_Library[i].Graphics.CenterCamera(graphicsArgs);
        m_Library[i].Graphics.ClearBackground(graphicsArgs);

        m_Library[i].Graphics.DrawGrid(
            { 0, 0 },
            m_Library[i].Grid.Data(),
            graphicsArgs
        );

        if (numAvailable % TemplatesPerRow != 0)
            ImGui::SameLine();
        cursorPos = ImGui::GetCursorPos();

        ImGui::Image(
            static_cast<ImTextureID>(m_Library[i].Graphics.TextureID()),
            { windowBounds.Width, windowBounds.Height},
            ImVec2(0, 1),
            ImVec2(1, 0)
        );
        ImGui::SetItemTooltip(std::format("{}.gol", m_Library[i].FileName).c_str());

        ImGui::SetCursorPos(cursorPos);

        if (ImGui::InvisibleButton(std::format("##{}", m_Library[i].FileName).c_str(), { windowBounds.Width, windowBounds.Height }))
        {
            retString = RLEEncoder::EncodeRegion(m_Library[i].Grid, {{0, 0}, m_Library[i].Grid.Size()}).c_str();
        }

        if (ImGui::IsItemHovered())
            m_Library[i].Graphics.DrawSelection({ {0, 0}, graphicsArgs.GridSize }, graphicsArgs);

        numAvailable++;
    }

    ImGui::End();
    return { .ClipboardText = retString };
}

void gol::PresetSelection::ReadFiles(const std::filesystem::path& path)
{
    m_MaxGridDimensions = Size2F {};
    for (const auto& file : std::filesystem::recursive_directory_iterator(path)) 
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

        std::string name = file.path().filename().string();
		name = name.substr(0, name.find_last_of('.'));
		m_MaxFileName = std::max(m_MaxFileName, name.length());
        m_Library.emplace_back(
            std::move(result->Grid),
            std::move(name),
            m_WindowSize
        );
    }
}