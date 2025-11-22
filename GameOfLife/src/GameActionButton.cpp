#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <span>
#include <string_view>

#include "GameActionButton.h"
#include "GameEnums.h"

gol::GameActionButton::GameActionButton(
	std::string_view label,
	GameAction actionReturn,
	std::span<const ImGuiKeyChord> shortcuts,
	bool lineBreak
)
	: m_Label(label)
	, m_Return(actionReturn)
	, m_Shortcuts(shortcuts.begin(), shortcuts.end())
	, m_LineBreak(lineBreak)
{ }

gol::GameAction gol::GameActionButton::Update(GameState state)
{
	if (!Enabled(state))
	{
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
	}

	if (!m_LineBreak)
		ImGui::SameLine();

	GameAction result = [&]()
	{
		bool active = false;
		for (auto& shortcut : m_Shortcuts)
			active = shortcut.Active() || active;

		if (ImGui::Button(m_Label.c_str(), Dimensions()) || (Enabled(state) && active))
			return m_Return;
		return GameAction::None;
	}();

	if (!Enabled(state))
	{
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}

	return result;
}