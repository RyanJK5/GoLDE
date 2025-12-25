#include <font-awesome/IconsFontAwesome7.h>
#include <imgui/imgui.h>
#include <optional>

#include "EditorWidget.h"
#include "GameEnums.h"
#include "SimulationControlResult.h"

gol::CopyButton::CopyButton(std::span<const ImGuiKeyChord> shortcuts)
    : ActionButton(SelectionAction::Copy, shortcuts)
{}

gol::Size2F gol::CopyButton::Dimensions() const
{
    return { ImGui::GetContentRegionAvail().x / 4.f, ActionButton::DefaultButtonHeight };
}

std::string gol::CopyButton::Label(const EditorResult&) const
{
    return ICON_FA_COPY;
}

bool gol::CopyButton::Enabled(const EditorResult& state) const
{
    return state.SelectionActive;
}

gol::CutButton::CutButton(std::span<const ImGuiKeyChord> shortcuts)
    : ActionButton(SelectionAction::Cut, shortcuts)
{}

gol::Size2F gol::CutButton::Dimensions() const
{
    return { ImGui::GetContentRegionAvail().x / 2.f, ActionButton::DefaultButtonHeight };
}

std::string gol::CutButton::Label(const EditorResult&) const
{
    return ICON_FA_SCISSORS;
}

bool gol::CutButton::Enabled(const EditorResult& state) const
{
    return state.SelectionActive && (state.State == SimulationState::Paint || state.State == SimulationState::Empty);
}

// PasteButton implementations
gol::PasteButton::PasteButton(std::span<const ImGuiKeyChord> shortcuts)
    : ActionButton(SelectionAction::Paste, shortcuts)
{}

gol::Size2F gol::PasteButton::Dimensions() const
{
    return { ImGui::GetContentRegionAvail().x / 3.f, ActionButton::DefaultButtonHeight };
}

std::string gol::PasteButton::Label(const EditorResult&) const
{
    return ICON_FA_PASTE;
}

bool gol::PasteButton::Enabled(const EditorResult& state) const
{
    return ImGui::GetClipboardText() && (state.State == SimulationState::Paint || state.State == SimulationState::Empty);
}

gol::DeleteButton::DeleteButton(std::span<const ImGuiKeyChord> shortcuts)
    : ActionButton(SelectionAction::Delete, shortcuts)
{}

gol::Size2F gol::DeleteButton::Dimensions() const
{
    return { ImGui::GetContentRegionAvail().x, ActionButton::DefaultButtonHeight };
}

std::string gol::DeleteButton::Label(const EditorResult&) const
{
    return ICON_FA_DELETE_LEFT;
}

bool gol::DeleteButton::Enabled(const EditorResult& state) const
{
    return state.SelectionActive && (state.State == SimulationState::Paint || state.State == SimulationState::Empty);
}

gol::DeselectButton::DeselectButton(std::span<const ImGuiKeyChord> shortcuts)
    : ActionButton(SelectionAction::Deselect, shortcuts)
{}

gol::Size2F gol::DeselectButton::Dimensions() const
{
    return { ImGui::GetContentRegionAvail().x / 4.f, ActionButton::DefaultButtonHeight };
}

std::string gol::DeselectButton::Label(const EditorResult&) const
{
    return ICON_FA_OBJECT_UNGROUP;
}

bool gol::DeselectButton::Enabled(const EditorResult& state) const
{
    return state.SelectionActive;
}

gol::RotateButton::RotateButton(std::span<const ImGuiKeyChord> shortcuts)
    : ActionButton(SelectionAction::Rotate, shortcuts)
{}

gol::Size2F gol::RotateButton::Dimensions() const
{
    return { ImGui::GetContentRegionAvail().x / 3.f, ActionButton::DefaultButtonHeight };
}

std::string gol::RotateButton::Label(const EditorResult&) const
{
    return ICON_FA_ROTATE;
}

bool gol::RotateButton::Enabled(const EditorResult& state) const
{
    return state.SelectionActive && (state.State == SimulationState::Paint || state.State == SimulationState::Empty);
}

gol::Size2F gol::FlipVerticalButton::Dimensions() const
{
    return { ImGui::GetContentRegionAvail().x / 2.f, ActionButton::DefaultButtonHeight };
}

std::string gol::FlipVerticalButton::Label(const EditorResult&) const
{
    return ICON_FA_ARROWS_UP_DOWN;
}

bool gol::FlipVerticalButton::Enabled(const EditorResult& state) const
{
    return state.SelectionActive && (state.State == SimulationState::Paint || state.State == SimulationState::Empty);
}

gol::FlipHorizontalButton::FlipHorizontalButton(std::span<const ImGuiKeyChord> shortcuts)
    : ActionButton(SelectionAction::FlipHorizontally, shortcuts)
{}

gol::Size2F gol::FlipHorizontalButton::Dimensions() const
{
    return { ImGui::GetContentRegionAvail().x, ActionButton::DefaultButtonHeight };
}

std::string gol::FlipHorizontalButton::Label(const EditorResult&) const
{
    return ICON_FA_ARROWS_LEFT_RIGHT;
}

bool gol::FlipHorizontalButton::Enabled(const EditorResult& state) const
{
    return state.SelectionActive && (state.State == SimulationState::Paint || state.State == SimulationState::Empty);
}

gol::FlipVerticalButton::FlipVerticalButton(std::span<const ImGuiKeyChord> shortcuts)
    : ActionButton(SelectionAction::FlipVertically, shortcuts)
{}

gol::UndoButton::UndoButton(std::span<const ImGuiKeyChord> shortcuts)
    : ActionButton(EditorAction::Undo, shortcuts)
{}

gol::Size2F gol::UndoButton::Dimensions() const
{
    return { ImGui::GetContentRegionAvail().x / 4.f, ActionButton::DefaultButtonHeight };
}

std::string gol::UndoButton::Label(const EditorResult&) const
{
    return ICON_FA_ARROW_ROTATE_LEFT;
}

bool gol::UndoButton::Enabled(const EditorResult& state) const
{
    return state.UndosAvailable && (state.State == SimulationState::Paint || state.State == SimulationState::Empty);
}

gol::RedoButton::RedoButton(std::span<const ImGuiKeyChord> shortcuts)
    : ActionButton(EditorAction::Redo, shortcuts)
{}

gol::Size2F gol::RedoButton::Dimensions() const
{
    return { ImGui::GetContentRegionAvail().x / 3.f, ActionButton::DefaultButtonHeight };
}

std::string gol::RedoButton::Label(const EditorResult&) const
{
    return ICON_FA_ARROW_ROTATE_RIGHT;
}

bool gol::RedoButton::Enabled(const EditorResult& state) const
{
    return state.RedosAvailable && (state.State == SimulationState::Paint || state.State == SimulationState::Empty);
}

gol::EditorWidget::EditorWidget(const std::unordered_map<ActionVariant, std::vector<ImGuiKeyChord>>& shortcuts)
	: m_CopyButton(shortcuts.at(SelectionAction::Copy))
	, m_CutButton(shortcuts.at(SelectionAction::Cut))
	, m_PasteButton(shortcuts.at(SelectionAction::Paste))
	, m_DeleteButton(shortcuts.at(SelectionAction::Delete))
	, m_DeselectButton(shortcuts.at(SelectionAction::Deselect))
	, m_RotateButton(shortcuts.at(SelectionAction::Rotate))
    , m_FlipHorizontalButton(shortcuts.at(SelectionAction::FlipHorizontally))
    , m_FlipVerticalButton(shortcuts.at(SelectionAction::FlipVertically))
	, m_UndoButton(shortcuts.at(EditorAction::Undo))
	, m_RedoButton(shortcuts.at(EditorAction::Redo))
{ }


gol::SimulationControlResult gol::EditorWidget::UpdateImpl(const EditorResult& state)
{
	auto result = SimulationControlResult {};
	
	UpdateResult(result, m_CopyButton.Update(state));
	UpdateResult(result, m_PasteButton.Update(state));
	UpdateResult(result, m_CutButton.Update(state));
	UpdateResult(result, m_DeleteButton.Update(state));

	UpdateResult(result, m_DeselectButton.Update(state));
    UpdateResult(result, m_RotateButton.Update(state));
    UpdateResult(result, m_FlipVerticalButton.Update(state));
    UpdateResult(result, m_FlipHorizontalButton.Update(state));

    UpdateResult(result, m_UndoButton.Update(state));

    ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 30.f);
	UpdateResult(result, m_RedoButton.Update(state));
    ImGui::Separator();
    ImGui::PopStyleVar();

    return result;
}
