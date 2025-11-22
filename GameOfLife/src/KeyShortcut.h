#ifndef __KeyShortcut_h__
#define __KeyShortcut_h__

#include <imgui/imgui.h>
#include <ranges>
#include <vector>

namespace gol
{
	class KeyShortcut
	{
	public:
		constexpr static auto MapChordsToVector =
			std::views::transform([](auto chord) { return KeyShortcut(chord); })
			| std::ranges::to<std::vector>();
	public:
		KeyShortcut(ImGuiKeyChord shortcut, bool onRelease = true) : m_Shortcut(shortcut), m_OnRelease(onRelease), m_Down(false) { }

		ImGuiKeyChord Shortcut() const { return m_Shortcut; }
		bool Active();
	private:
		ImGuiKeyChord m_Shortcut;
		bool m_OnRelease;
		bool m_Down;
	};
}

#endif