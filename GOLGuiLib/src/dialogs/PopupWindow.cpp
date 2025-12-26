#include <string_view>

#include <imgui/imgui.h>

#include "PopupWindow.h"

gol::PopupWindow::PopupWindow(std::string_view title, std::string_view message)
	: m_Title(title)
	, Message(message)
{ }

gol::PopupWindowState gol::PopupWindow::Update()
{
	if (!Active)
		return PopupWindowState::None;

	ImGui::OpenPopup(m_Title.c_str());
	ImGui::BeginPopupModal(m_Title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

	ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 30.f);
	ImGui::Text(Message.c_str());
	ImGui::PopStyleVar();

	auto result = ShowButtons();
	if (result != PopupWindowState::None)
		Active = false;
	
	ImGui::EndPopup();
	return result;
}