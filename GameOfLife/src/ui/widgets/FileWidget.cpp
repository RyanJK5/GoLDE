#include <expected>
#include <filesystem>
#include <font-awesome/IconsFontAwesome7.h>
#include <imgui.h>
#include <span>
#include <string>

#include "ActionButton.h"
#include "FileDialog.h"
#include "FileWidget.h"
#include "GameEnums.h"
#include "Graphics2D.h"
#include "PopupWindow.h"
#include "SimulationControlResult.h"

gol::UpdateFileButton::UpdateFileButton(std::span<const ImGuiKeyChord> shortcuts) : ActionButton(EditorAction::UpdateFile, shortcuts) { }
gol::Size2F     gol::UpdateFileButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x / 4.f, ActionButton::DefaultButtonHeight }; }
std::string     gol::UpdateFileButton::Label(const EditorResult&) const { return ICON_FA_FILE_ARROW_UP; }
bool            gol::UpdateFileButton::Enabled(const EditorResult& state) const { return state.State != SimulationState::Simulation; }

gol::SaveButton::SaveButton(std::span<const ImGuiKeyChord> shortcuts) : ActionButton(EditorAction::Save, shortcuts) { }
gol::Size2F     gol::SaveButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x / 3.f, ActionButton::DefaultButtonHeight }; }
std::string     gol::SaveButton::Label(const EditorResult&) const { return ICON_FA_FLOPPY_DISK; }
bool            gol::SaveButton::Enabled(const EditorResult& state) const { return state.State == SimulationState::Paint || state.State == SimulationState::Paused; }

gol::LoadButton::LoadButton(std::span<const ImGuiKeyChord> shortcuts) : ActionButton(EditorAction::Load, shortcuts) {}
gol::Size2F     gol::LoadButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x / 2.f, ActionButton::DefaultButtonHeight }; }
std::string     gol::LoadButton::Label(const EditorResult&) const { return ICON_FA_FOLDER_OPEN; }
bool            gol::LoadButton::Enabled(const EditorResult& state) const { return state.State != SimulationState::Simulation; }

gol::FileWidget::FileWidget(std::span<const ImGuiKeyChord> updateFileShortcuts, 
		std::span<const ImGuiKeyChord> saveShortcuts, std::span<const ImGuiKeyChord> loadShortcuts)
	: m_UpdateFileButton(updateFileShortcuts)
	, m_SaveButton(saveShortcuts)
	, m_LoadButton(loadShortcuts)
	, m_FileNotOpened("File Not Opened")
{ }

gol::SimulationControlResult gol::FileWidget::UpdateImpl(const EditorResult& state)
{
	auto popupState = m_FileNotOpened.Update();
	if (popupState == PopupWindowState::Success)
		m_FileNotOpened.Active = false;

	auto result = m_UpdateFileButton.Update(state);

	auto saveResult = m_SaveButton.Update(state);
	if (!result.Action)
		result = saveResult;

	ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 30.f);
	auto loadResult = m_LoadButton.Update(state);
	ImGui::Separator();
	ImGui::PopStyleVar();

	if (!result.Action)
		result = loadResult;
	if (!result.Action)
		return { .FilePath = state.CurrentFilePath };
	
	auto filePath = [&result, state] -> std::expected<std::filesystem::path, FileDialogFailure>
	{
		switch (*result.Action)
		{
		using enum EditorAction;
		case UpdateFile:
			if (!state.CurrentFilePath.empty())
				return state.CurrentFilePath;
			return FileDialog::SaveFileDialog("gol", "");
		case Save:
			return FileDialog::SaveFileDialog("gol", state.CurrentFilePath.string());
		case Load:
			return FileDialog::OpenFileDialog("gol", "");
		}
		return std::unexpected { FileDialogFailure { FileFailureType::Error, "Unknown action" } };
	}();

	if (!filePath)
	{
		if (filePath.error().Type == FileFailureType::Error)
		{
			m_FileNotOpened.Active = true;
			m_FileNotOpened.Message = filePath.error().Message;
		}
		return { .FilePath = state.CurrentFilePath };
	}

	return { .Action = result.Action, .FilePath = *filePath, .FromShortcut = result.FromShortcut };
}