#include "GameActionButton.h"

#include "imgui.h"
#include <imgui_internal.h>

gol::GameActionButton::GameActionButton(
	std::string_view label,
	GameAction actionReturn,
	const std::function<Size2F()>& dimensions,
	const std::function<bool(GameState)>& enabledCheck,
	const std::vector<ImGuiKeyChord>& shortcuts,
	bool lineBreak
)
	: m_Label(label)
	, m_Return(actionReturn)
	, m_Size(dimensions)
	, m_Enabled(enabledCheck)
	, m_Shortcuts()
	, m_LineBreak(lineBreak)
{
	for (auto& chord : shortcuts)
		m_Shortcuts.emplace_back(chord);
}

gol::GameAction gol::GameActionButton::Update(GameState state)
{
	if (!m_Enabled(state))
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

		if (ImGui::Button(m_Label.c_str(), m_Size()) || (m_Enabled(state) && active))
			return m_Return;
		return GameAction::None;
	}();

	if (!m_Enabled(state))
	{
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}

	return result;
}