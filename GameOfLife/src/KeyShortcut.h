#ifndef __KeyShortcut_h__
#define __KeyShortcut_h__

#include <unordered_map>

#include "vendor/imgui.h"

namespace gol
{
	class KeyShortcut
	{
	public:
		KeyShortcut(ImGuiKeyChord shortcut, bool onRelease = true) : m_Shortcut(shortcut), m_OnRelease(onRelease), m_Down(false) { }

		bool Active();
	private:
		ImGuiKeyChord m_Shortcut;
		bool m_OnRelease;
		bool m_Down;
	};
}

#endif