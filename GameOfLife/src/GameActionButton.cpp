#include "GameActionButton.h"

#include "vendor/imgui.h"
#include "vendor/imgui_internal.h"

gol::GameActionButton::GameActionButton(
	std::string_view label,
	GameAction actionReturn,
	Size2F size,
	const std::function<bool(const DrawInfo&)>& enabledCheck,
	const std::vector<ImGuiKeyChord>& shortcuts,
	bool lineBreak
)
	: m_Label(label)
	, m_Return(actionReturn)
	, m_Size(size)
	, m_Enabled(enabledCheck)
	, m_Shortcuts()
	, m_LineBreak(lineBreak)
{
	for (auto& chord : shortcuts)
	{
		m_Shortcuts.emplace_back(chord);
	}
}

gol::GameAction gol::GameActionButton::Update(const DrawInfo& info)
{
	if (!m_Enabled(info))
	{
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
	}

	if (!m_LineBreak)
		ImGui::SameLine();

	GameAction result = [&]()
	{
		bool active = m_Shortcuts.size() > 0;
		for (auto& shortcut : m_Shortcuts)
		{
			active = shortcut.Active() && active;
		}

		if (ImGui::Button(m_Label.data(), {m_Size.Width, m_Size.Height}) || (m_Enabled(info) && active))
			return m_Return;
		return GameAction::None;
	}();

	if (!m_Enabled(info))
	{
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}

	return result;
}