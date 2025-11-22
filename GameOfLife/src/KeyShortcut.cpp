#include <imgui/imgui.h>

#include "KeyShortcut.h"

bool gol::KeyShortcut::Active()
{
	bool keyCombo = ImGui::IsKeyChordPressed(m_Shortcut);
	bool result = (!m_OnRelease && keyCombo) || (m_Down && !keyCombo);

	if (keyCombo)
		m_Down = true;
	else if (m_Down)
		m_Down = false;

	return result;
}