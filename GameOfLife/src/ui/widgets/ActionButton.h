#ifndef __GameActionButton_h__
#define __GameActionButton_h__

#include <cstdint>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

#include <print>

#include "GameEnums.h"
#include "Graphics2D.h"
#include "KeyShortcut.h"

namespace gol
{
	template <ActionType ActType, bool LineBreak>
	class MultiActionButton
	{
	public:
		static constexpr int32_t DefaultButtonHeight = 50;

		MultiActionButton(const std::unordered_map<ActType, std::vector<KeyShortcut>>& shortcuts)
			: m_Shortcuts(shortcuts)
		{ }

		std::optional<ActType> Update(EditorState state)
		{
			if (!Enabled(state))
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (!m_LineBreak)
				ImGui::SameLine();

			auto result = [this, state]() -> std::optional<ActType>
			{
				bool active = false;
				if (Enabled(state))
					for (auto& shortcut : m_Shortcuts.at(Action(state)))
						active = shortcut.Active() || active;

				if (ImGui::Button(Label(state).c_str(), Dimensions()) || active)
					return Action(state);
				return std::nullopt;
			}();
			
			if (!Enabled(state))
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
			else if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			{
				auto tooltip = Actions::ToString(Action(state));
				const auto& shortcuts = m_Shortcuts.at(Action(state));
				if (!shortcuts.empty())
					tooltip += ": " + KeyShortcut::StringRepresentation(m_Shortcuts.at(Action(state)));
				ImGui::SetTooltip(tooltip.c_str());
			}

			return result;
		}
	protected:
		virtual ActType Action(EditorState state) const = 0;
		
		virtual Size2F Dimensions() const = 0;
		virtual std::string Label(EditorState state) const = 0;
		virtual	bool Enabled(EditorState state) const = 0;
	private:
		bool m_LineBreak = LineBreak;

		std::unordered_map<ActType, std::vector<KeyShortcut>> m_Shortcuts;
	};

	template <ActionType ActType, bool LineBreak>
	class ActionButton : public MultiActionButton<ActType, LineBreak>
	{
	public:
		ActionButton(ActType action, std::span<const ImGuiKeyChord> shortcuts) 
			: MultiActionButton<ActType, LineBreak>({{action, shortcuts | KeyShortcut::MapChordsToVector}}), m_Action(action)
		{ }
	protected:
		virtual ActType Action(EditorState) const override final { return m_Action; }
		
		virtual Size2F Dimensions() const = 0;
		virtual std::string Label(EditorState state) const = 0;
		virtual	bool Enabled(EditorState state) const = 0;
	private:
		ActType m_Action;
	};
}

#endif