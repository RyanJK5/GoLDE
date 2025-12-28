#include <cmath>
#include <cstdint>
#include <filesystem>
#include <format>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <limits>
#include <locale>
#include <optional>
#include <string>
#include <utility>
#include <variant>

#include "GameEnums.h"
#include "GameGrid.h"
#include "GraphicsHandler.h"
#include "Graphics2D.h"
#include "Logging.h"
#include "SimulationControlResult.h"
#include "SimulationEditor.h"
#include "VersionManager.h"
#include "PopupWindow.h"
#include "PresetSelectionResult.h"

gol::SimulationEditor::SimulationEditor(uint32_t id, const std::filesystem::path& path, Size2 windowSize, Size2 gridSize)
    : m_EditorID(id)
    , m_CurrentFilePath(path)
    , m_Grid(gridSize)
    , m_Graphics(
        std::filesystem::path("resources") / "shader", 
        windowSize.Width, windowSize.Height,
        { 0.1f, 0.1f, 0.1f, 1.f }
    )
    , m_PasteWarning("Paste Warning")
    , m_FileErrorWindow("File Error")
{ }   

bool gol::SimulationEditor::IsSaved() const
{
    return m_VersionManager.IsSaved();
}


bool gol::SimulationEditor::operator==(const SimulationEditor& other) const
{
    return m_EditorID == other.m_EditorID;
}

gol::EditorResult gol::SimulationEditor::Update(std::optional<bool> activeOverride, const SimulationControlResult& controlArgs, const PresetSelectionResult& presetArgs)
{
    auto displayResult = DisplaySimulation(controlArgs.Action && activeOverride && (*activeOverride));
    if (!displayResult.Visible || (activeOverride && !(*activeOverride)))
        return { .Active = false, .Closing = displayResult.Closing };

    const auto graphicsArgs = GraphicsHandlerArgs
    { 
        .ViewportBounds = ViewportBounds(), 
        .GridSize = m_Grid.Size(),
		.CellSize = { SimulationEditor::DefaultCellWidth, SimulationEditor::DefaultCellHeight },
		.ShowGridLines = controlArgs.GridLines
    };
    UpdateViewport();
    UpdateDragState();
    m_Graphics.RescaleFrameBuffer(WindowBounds(), ViewportBounds());
    m_Graphics.ClearBackground(graphicsArgs);

    auto pasteWarnResult = m_PasteWarning.Update();
    if (pasteWarnResult == PopupWindowState::Success)
    {
        auto pasteResult = m_SelectionManager.Paste(CursorGridPos(), std::numeric_limits<uint32_t>::max());
        if (pasteResult)
            m_VersionManager.PushChange(*pasteResult);
    }
    m_FileErrorWindow.Update();
    
    if (presetArgs.ClipboardText.length() > 0) 
    {
		ImGui::SetClipboardText(presetArgs.ClipboardText.c_str());
        m_VersionManager.TryPushChange(m_SelectionManager.Deselect(m_Grid));
        auto result = m_SelectionManager.Paste(Vec2{ 0, 0 }, std::numeric_limits<uint32_t>::max(), true);
        if (result)
            m_VersionManager.PushChange(*result);
	}

    if (controlArgs.TickDelayMs)
        m_TickDelayMs = *controlArgs.TickDelayMs;

    if (controlArgs.Action && ((activeOverride && *activeOverride) || displayResult.Selected))
        m_State = UpdateState(controlArgs);

    m_State = [this, &graphicsArgs]()
    {
        switch (m_State)
        {
        using enum SimulationState;
        case Simulation:
            return SimulationUpdate(graphicsArgs);
        case Paint:
            return PaintUpdate(graphicsArgs);
        case Paused:
            return PauseUpdate(graphicsArgs);
        case Empty:
            return PaintUpdate(graphicsArgs);
        case None:
            return Empty;
        };
        std::unreachable();
    }();

    return 
    { 
        .CurrentFilePath = m_CurrentFilePath,
        .State = m_State,
        .Active = (activeOverride && *activeOverride) || displayResult.Selected,
		.Closing = displayResult.Closing || controlArgs.Action == ActionVariant { EditorAction::Close },
        .SelectionActive = m_SelectionManager.CanDrawGrid(),
		.UndosAvailable = m_VersionManager.UndosAvailable(),
		.RedosAvailable = m_VersionManager.RedosAvailable(),
		.HasUnsavedChanges = !m_VersionManager.IsSaved()
    };
}

gol::SimulationState gol::SimulationEditor::SimulationUpdate(const GraphicsHandlerArgs& args)
{
    GL_DEBUG(auto elapsed = (glfwGetTime() - m_LastTime) * 1000);
    if (elapsed >= m_TickDelayMs)
    {
        GL_DEBUG(m_LastTime = glfwGetTime());
        m_Grid.Update();
        if (m_Grid.Dead() && !m_SelectionManager.GridAlive())
            return SimulationState::Empty;
    }
    m_Graphics.DrawGrid({ 0, 0 }, m_Grid.Data(), args);
    return SimulationState::Simulation;
}

gol::SimulationState gol::SimulationEditor::PaintUpdate(const GraphicsHandlerArgs& args)
{
    auto gridPos = CursorGridPos();
    
    m_Graphics.DrawGrid({ 0, 0 }, m_Grid.Data(), args);
    if (m_SelectionManager.CanDrawGrid())
        m_Graphics.DrawGrid(m_SelectionManager.SelectionBounds().UpperLeft(), m_SelectionManager.GridData(), args);
    if (m_SelectionManager.CanDrawGrid())
        m_Graphics.DrawSelection(m_SelectionManager.SelectionBounds(), args);

    if (gridPos)
    {
        if (m_SelectionManager.CanDrawSelection())
            m_Graphics.DrawSelection(m_SelectionManager.SelectionBounds(), args);
        UpdateMouseState(*gridPos);
    }
    else
    {
        m_SelectionManager.TryResetSelection();
    }
    m_LeftDeltaLast = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);

    return (m_Grid.Dead() && !m_SelectionManager.GridAlive())
        ? SimulationState::Empty 
        : SimulationState::Paint;
}

gol::SimulationState gol::SimulationEditor::PauseUpdate(const GraphicsHandlerArgs& args)
{
    auto gridPos = CursorGridPos();
    if (gridPos)
        m_VersionManager.TryPushChange(m_SelectionManager.UpdateSelectionArea(m_Grid, *gridPos).Change);
    m_Graphics.DrawGrid({ 0, 0 }, m_Grid.Data(), args);
    if (m_SelectionManager.CanDrawSelection())
        m_Graphics.DrawSelection(m_SelectionManager.SelectionBounds(), args);
    if (m_SelectionManager.CanDrawGrid())
        m_Graphics.DrawGrid(m_SelectionManager.SelectionBounds().UpperLeft(), m_SelectionManager.GridData(), args);
    return SimulationState::Paused;
}

gol::SimulationEditor::DisplayResult gol::SimulationEditor::DisplaySimulation(bool grabFocus)
{
    auto label = std::format("{}{}###Simulation{}",
        m_CurrentFilePath.empty() ? "(untitled)" : m_CurrentFilePath.filename().string().c_str(),
        (!m_CurrentFilePath.empty() && !m_VersionManager.IsSaved()) ? "*" : "",
        m_EditorID
    );
    
    bool stayOpen = true;
    auto windowClass = ImGuiWindowClass{};
    windowClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoCloseButton;
    ImGui::SetNextWindowClass(&windowClass);
    if (!ImGui::Begin(label.c_str(), &stayOpen))
    {
        ImGui::End();
        return { .Visible = false, .Closing = !stayOpen };
    }
    bool windowFocused = ImGui::IsWindowFocused();
    ImGui::BeginChild("Render");

    ImDrawListSplitter splitter {};
    splitter.Split(ImGui::GetWindowDrawList(), 2);

    splitter.SetCurrentChannel(ImGui::GetWindowDrawList(), 0);
    m_WindowBounds = { Vec2F(ImGui::GetWindowPos()), Size2F(ImGui::GetContentRegionAvail()) };

    ImGui::Image(
        static_cast<ImTextureID>(m_Graphics.TextureID()),
        ImGui::GetContentRegionAvail(),
        ImVec2(0, 1),
        ImVec2(1, 0)
    );
    ImGui::SetCursorPosY(0);
    ImGui::InvisibleButton("##SimulationViewport", ImGui::GetContentRegionAvail());
    
    if (grabFocus || (m_TakeKeyboardInput && !ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Escape)))
        ImGui::SetKeyboardFocusHere(-1);

    m_TakeKeyboardInput = ImGui::IsItemFocused() || windowFocused;
    m_TakeMouseInput = ImGui::IsItemHovered();

    splitter.SetCurrentChannel(ImGui::GetWindowDrawList(), 1);
    ImGui::SetCursorPos(ImGui::GetWindowContentRegionMin());
    ImGui::Text(std::format("Generation: {}", m_Grid.Generation()).c_str());
    ImGui::Text(std::format("Population: {}", m_Grid.Population() + m_SelectionManager.SelectedPopulation()).c_str());

    if (m_SelectionManager.CanDrawSelection())
    {
        Vec2 pos = m_SelectionManager.SelectionBounds().UpperLeft();
        std::string text = std::format("({}, {})", pos.X, pos.Y);
        if (m_SelectionManager.CanDrawLargeSelection())
        {
            auto sentinel = m_SelectionManager.SelectionBounds().LowerRight();
            text += std::format(" X ({}, {})", sentinel.X, sentinel.Y);
        }
        ImGui::SetCursorPosY(ImGui::GetContentRegionMax().y - ImGui::CalcTextSize(text.c_str()).y);
        ImGui::Text(text.c_str());
    }

    splitter.Merge(ImGui::GetWindowDrawList());
    ImGui::EndChild();
    ImGui::End();

    return { .Visible = true, .Selected = m_TakeKeyboardInput, .Closing = !stayOpen };
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

std::optional<gol::Vec2> gol::SimulationEditor::ConvertToGridPos(Vec2F screenPos)
{
    if (!ViewportBounds().InBounds(screenPos.X, screenPos.Y))
        return std::nullopt;

    glm::vec2 vec = m_Graphics.Camera.ScreenToWorldPos(screenPos, ViewportBounds());
    vec /= glm::vec2 { DefaultCellWidth, DefaultCellHeight };
    
    Vec2 result = { static_cast<int32_t>(std::floor(vec.x)), static_cast<int32_t>(std::floor(vec.y)) };
    if (!m_Grid.InBounds(result))
        return std::nullopt;
    return result;
}

std::optional<gol::Vec2> gol::SimulationEditor::CursorGridPos()
{
    return ConvertToGridPos(ImGui::GetMousePos());
}

void gol::SimulationEditor::UpdateVersion(const SimulationControlResult& args)
{
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
        return;

	auto action = std::get<EditorAction>(*args.Action);

    auto versionChanges = action == EditorAction::Undo
        ? m_VersionManager.Undo()
		: m_VersionManager.Redo();
    if (!versionChanges)
        return;

	m_SelectionManager.HandleVersionChange(action, m_Grid, *versionChanges);
}

gol::SimulationState gol::SimulationEditor::UpdateState(const SimulationControlResult& result)
{
    if (result.FromShortcut && !m_TakeKeyboardInput)
        return result.State;

    if (auto* action = std::get_if<GameAction>(&*result.Action))
    {
        switch(*action)
        {
        using enum GameAction;
        case Start:
            m_SelectionManager.Deselect(m_Grid);
            m_InitialGrid = m_Grid;
            return SimulationState::Simulation;
        case Clear:
            m_VersionManager.TryPushChange(m_SelectionManager.Deselect(m_Grid));
            m_VersionManager.PushChange({
                .Action = GameAction::Clear,
                .SelectionBounds = m_Grid.BoundingBox(),
                .CellsInserted = {},
                .CellsDeleted = m_Grid.Data()
            });
            m_Grid = GameGrid { m_Grid.Size() };
            return SimulationState::Paint;
        case Reset:
            m_SelectionManager.Deselect(m_Grid);
            m_Grid = m_InitialGrid;
            return SimulationState::Paint;
        case Restart:
            m_SelectionManager.Deselect(m_Grid);
            m_Grid = m_InitialGrid;
            return SimulationState::Simulation;
        case Pause:
            return SimulationState::Paused;
        case Resume:
            m_SelectionManager.Deselect(m_Grid);
            return SimulationState::Simulation;
        case Step:
            m_SelectionManager.Deselect(m_Grid);
            if (result.State == SimulationState::Paint)
                m_InitialGrid = m_Grid;
            for (int32_t i = 0; i < *result.StepCount; i++)
                m_Grid.Update();
            return m_Grid.Dead() ? SimulationState::Empty : SimulationState::Paused;
        }
    }

    if (auto* action = std::get_if<EditorAction>(&*result.Action))
    {
        switch (*action)
        {
        using enum EditorAction;
        case Resize:
            return ResizeGrid(result);
        case Undo:
            UpdateVersion(result);
            return result.State;
        case Redo:
            UpdateVersion(result);
            return result.State;
        case UpdateFile: [[fallthrough]];
        case Save:
            if (m_SelectionManager.Save(m_Grid, *result.FilePath))
            {
                if (m_CurrentFilePath.empty())
                    m_CurrentFilePath = *result.FilePath;
                if (*action == UpdateFile)
                    m_VersionManager.Save();
            }
            else
            {
                m_FileErrorWindow.Active = true;
                m_FileErrorWindow.Message = std::format(
                    "Failed to save file to \n{}"
                    , result.FilePath->string()
                );
            }
            return result.State;
        case Load:
            LoadFile(result);
            m_VersionManager.Save();
            return result.State;
        case Close:
            return result.State;
        }
	}

    if (auto* action = std::get_if<SelectionAction>(&*result.Action))
    {
        if (*action == SelectionAction::Paste)
        {
            PasteSelection();
            return result.State;
        }
        m_VersionManager.TryPushChange(m_SelectionManager.HandleAction(*action, m_Grid, result.NudgeSize));
    }

    return result.State;
}

void gol::SimulationEditor::PasteSelection()
{
    auto gridPos = CursorGridPos();
    if (gridPos)
        m_VersionManager.TryPushChange(m_SelectionManager.Deselect(m_Grid));
    auto pasteResult = m_SelectionManager.Paste(gridPos, 1000000U);
    if (pasteResult)
        m_VersionManager.PushChange(*pasteResult);
    else if (pasteResult.error() != 0)
    {
        m_PasteWarning.Active = true;
        m_PasteWarning.Message = std::format(
            std::locale(""),
            "Your selection ({:L} cells) is too large\n"
            "to paste without potential performance issues.\n"
            "Are you sure you want to continue?"
            , pasteResult.error())
            ;
    }
}

void gol::SimulationEditor::LoadFile(const gol::SimulationControlResult& result)
{
    m_VersionManager.TryPushChange(m_SelectionManager.Deselect(m_Grid));

    auto loadResult = m_SelectionManager.Load(*result.FilePath);
    if (loadResult)
        m_VersionManager.PushChange(*loadResult);
    else
    {
        m_FileErrorWindow.Active = true;
        m_FileErrorWindow.Message = std::format(
            "Failed to load file:\n{}"
            , loadResult.error()
        );
    }
}

gol::SimulationState gol::SimulationEditor::ResizeGrid(const gol::SimulationControlResult& result)
{
	if (result.NewDimensions->Width == m_Grid.Width() && result.NewDimensions->Height == m_Grid.Height())
        return SimulationState::Paint;
    if ((result.NewDimensions->Width == 0 || result.NewDimensions->Height == 0) &&
            (m_Grid.Width() == 0 || m_Grid.Height() == 0))
        return SimulationState::Paint;

    m_VersionManager.PushChange
    ({
        .Action = EditorAction::Resize,
        .GridResize = { { m_Grid, *result.NewDimensions } }
    });

    m_Grid = GameGrid(std::move(m_Grid), *result.NewDimensions);
    if (m_SelectionManager.CanDrawSelection())
    {
        auto selection = m_SelectionManager.SelectionBounds();
        if (!m_Grid.InBounds(selection.UpperLeft()) || !m_Grid.InBounds(selection.UpperRight()) ||
            !m_Grid.InBounds(selection.LowerLeft()) || !m_Grid.InBounds(selection.LowerRight()))
            m_VersionManager.TryPushChange(m_SelectionManager.Deselect(m_Grid));
    }
    m_Graphics.Camera.Center =
    {
        result.NewDimensions->Width * DefaultCellWidth / 2.f,
        result.NewDimensions->Height * DefaultCellHeight / 2.f
    };
    return SimulationState::Paint;
}

void gol::SimulationEditor::UpdateMouseState(Vec2 gridPos)
{
    if (!m_TakeMouseInput || ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId)) 
        return;

    auto result = m_SelectionManager.UpdateSelectionArea(m_Grid, gridPos);
    if (result.BeginSelection)
    {
        m_EditorMode = EditorMode::Select;
        return;
    }
    m_VersionManager.TryPushChange(result.Change);

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && !ImGui::IsKeyDown(ImGuiKey_LeftShift) && !ImGui::IsKeyDown(ImGuiKey_RightShift))
    {
        if (m_EditorMode == EditorMode::None)
        {
            m_EditorMode = *m_Grid.Get(gridPos.X, gridPos.Y) ? EditorMode::Delete : EditorMode::Insert;
            m_VersionManager.BeginPaintChange(gridPos, m_EditorMode == EditorMode::Insert);
            m_LeftDeltaLast = {};
        }
		
        FillCells();
        return;
    }
    m_EditorMode = EditorMode::None;
}

void gol::SimulationEditor::FillCells()
{
    const auto mousePos = Vec2F { ImGui::GetMousePos() };
    const auto delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
    
    const auto realDelta = Vec2F { delta.x - m_LeftDeltaLast.X, delta.y - m_LeftDeltaLast.Y };
    const auto lastPos = mousePos - realDelta;
    const auto deltaStep = realDelta.Normalized();

    for (float i = 0; i <= realDelta.Magnitude(); i++)
    {
        auto pos = Vec2F { lastPos.X + i * deltaStep.X, lastPos.Y + i * deltaStep.Y };
        auto gridPos = ConvertToGridPos(pos);
        if (!gridPos)
			break;

        if (*m_Grid.Get(gridPos->X, gridPos->Y) != (m_EditorMode == EditorMode::Insert))
            m_VersionManager.AddPaintChange(*gridPos);
		m_Grid.Set(gridPos->X, gridPos->Y, m_EditorMode == EditorMode::Insert);
    }
}

void gol::SimulationEditor::UpdateDragState()
{
    if (!m_TakeMouseInput || !ImGui::IsMouseDragging(ImGuiMouseButton_Right))
    {
        m_RightDeltaLast = { 0.f, 0.f };
        return;
    }

    Vec2F delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
    m_Graphics.Camera.Translate(glm::vec2(delta) - glm::vec2(m_RightDeltaLast));
    m_RightDeltaLast = delta;
}

void gol::SimulationEditor::UpdateViewport()
{
    const Rect bounds = ViewportBounds();
    
    auto mousePos = ImGui::GetIO().MousePos;
    if (bounds.InBounds(mousePos.x, mousePos.y) && ImGui::GetIO().MouseWheel != 0)
        m_Graphics.Camera.ZoomBy(mousePos, bounds, ImGui::GetIO().MouseWheel / 10.f);
}