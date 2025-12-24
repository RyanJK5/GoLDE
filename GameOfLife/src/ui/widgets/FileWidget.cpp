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

gol::NewFileButton::NewFileButton(std::span<const ImGuiKeyChord> shortcuts) : ActionButton(EditorAction::NewFile, shortcuts) {}
gol::Size2F     gol::NewFileButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x / 4.f, ActionButton::DefaultButtonHeight }; }
std::string     gol::NewFileButton::Label(const EditorResult&) const { return ICON_FA_FILE_CIRCLE_PLUS; }
bool            gol::NewFileButton::Enabled(const EditorResult&) const { return true; }

gol::UpdateFileButton::UpdateFileButton(std::span<const ImGuiKeyChord> shortcuts) : ActionButton(EditorAction::UpdateFile, shortcuts) { }
gol::Size2F     gol::UpdateFileButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x / 3.f, ActionButton::DefaultButtonHeight }; }
std::string     gol::UpdateFileButton::Label(const EditorResult&) const { return ICON_FA_FILE_ARROW_UP; }
bool            gol::UpdateFileButton::Enabled(const EditorResult& state) const 
{ 
	return (state.CurrentFilePath.empty() && state.State == SimulationState::Paused) || state.State == SimulationState::Paint || state.State == SimulationState::Empty; 
}

gol::SaveButton::SaveButton(std::span<const ImGuiKeyChord> shortcuts) : ActionButton(EditorAction::Save, shortcuts) { }
gol::Size2F     gol::SaveButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x / 2.f, ActionButton::DefaultButtonHeight }; }
std::string     gol::SaveButton::Label(const EditorResult&) const { return ICON_FA_FLOPPY_DISK; }
bool            gol::SaveButton::Enabled(const EditorResult& state) const { return state.State == SimulationState::Paint || state.State == SimulationState::Paused; }

gol::LoadButton::LoadButton(std::span<const ImGuiKeyChord> shortcuts) : ActionButton(EditorAction::Load, shortcuts) {}
gol::Size2F     gol::LoadButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x, ActionButton::DefaultButtonHeight }; }
std::string     gol::LoadButton::Label(const EditorResult&) const { return ICON_FA_FOLDER_OPEN; }
bool            gol::LoadButton::Enabled(const EditorResult&) const { return true; }

gol::FileWidget::FileWidget(const std::unordered_map<ActionVariant, std::vector<ImGuiKeyChord>>& shortcutInfo)
	: m_NewFileButton(shortcutInfo.at(EditorAction::NewFile))
	, m_UpdateFileButton(shortcutInfo.at(EditorAction::UpdateFile))
	, m_SaveButton(shortcutInfo.at(EditorAction::Save))
	, m_LoadButton(shortcutInfo.at(EditorAction::Load))
	, m_FileNotOpened("File Not Opened")
{ }

gol::SimulationControlResult gol::FileWidget::UpdateImpl(const EditorResult& state)
{
	auto popupState = m_FileNotOpened.Update();
	if (popupState == PopupWindowState::Success)
		m_FileNotOpened.Active = false;

	auto result = SimulationControlResult{};
	UpdateResult(result, m_NewFileButton.Update(state));
	UpdateResult(result, m_UpdateFileButton.Update(state));
	UpdateResult(result, m_SaveButton.Update(state));

	ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 30.f);
	UpdateResult(result, m_LoadButton.Update(state));
	ImGui::Separator();
	ImGui::PopStyleVar();

	if (!result.Action)
		return { .FilePath = state.CurrentFilePath };
	
	auto filePath = [&result, state] -> std::expected<std::filesystem::path, FileDialogFailure>
	{
		switch (std::get<EditorAction>(*result.Action))
		{
		using enum EditorAction;
		case NewFile:
			return std::filesystem::path{};
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