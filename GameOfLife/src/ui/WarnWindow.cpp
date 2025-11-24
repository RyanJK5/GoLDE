#include <cstdint>
#include <imgui/imgui.h>
#include <string>

#include "ActionButton.h"
#include "GameEnums.h"
#include "WarnWindow.h"

gol::WarnWindowState gol::WarnWindow::Update()
{
	if (!Active)
		return WarnWindowState::None;

	ImGui::OpenPopup(m_Title.c_str());
	ImGui::BeginPopupModal(m_Title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

	constexpr int32_t height = ActionButton<EditorAction, false>::DefaultButtonHeight;
	ImGui::Text(Message.c_str());
	bool yes = ImGui::Button("Yes", 
	{ 
		ImGui::GetContentRegionAvail().x,
		height
	});
	bool no = ImGui::Button("No",
	{
		ImGui::GetContentRegionAvail().x,
		height
	});
	
	ImGui::EndPopup();

	if (yes)
		return WarnWindowState::Success;
	if (no)
		return WarnWindowState::Failure;
	return WarnWindowState::None;
}