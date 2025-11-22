#ifndef __SimulationControl_h__
#define __SimulationControl_h__

#include <cstdint>
#include <imgui/imgui.h>
#include <ranges>
#include <unordered_map>
#include <vector>

#include "DelayWidget.h"
#include "EditorWidget.h"
#include "ExecutionWidget.h"
#include "GameEnums.h"
#include "GUILoader.h"
#include "KeyShortcut.h"
#include "ResizeWidget.h"
#include "SimulationControlResult.h"
#include "StepWidget.h"
#include "VersionManager.h"

namespace gol
{
	struct SelectionShortcuts
	{
		std::unordered_map<GameAction, std::vector<KeyShortcut>> Shortcuts;

		SelectionShortcuts(
			const std::vector<ImGuiKeyChord>& left, 
			const std::vector<ImGuiKeyChord>& right, 
			const std::vector<ImGuiKeyChord>& up, 
			const std::vector<ImGuiKeyChord>& down,
			const std::vector<ImGuiKeyChord>& deselect,
			const std::vector<ImGuiKeyChord>& rotate
		)
			: Shortcuts({
				{ GameAction::NudgeLeft,  left      | KeyShortcut::MapChordsToVector },
				{ GameAction::NudgeRight, right     | KeyShortcut::MapChordsToVector },
				{ GameAction::NudgeUp,    up        | KeyShortcut::MapChordsToVector },
				{ GameAction::NudgeDown,  down      | KeyShortcut::MapChordsToVector },
				{ GameAction::Deselect,   deselect  | KeyShortcut::MapChordsToVector },
				{ GameAction::Rotate,     rotate    | KeyShortcut::MapChordsToVector },
			})
		{ }

		SimulationControlResult Update(GameState state);
	};

	class SimulationControl
	{
	public:
		SimulationControl(const StyleLoader::StyleInfo<ImVec4>& fileInfo);

		SimulationControlResult Update(GameState state);
	private:
		void FillResults(SimulationControlResult& current, const SimulationControlResult& update) const;
	private:
		static constexpr int32_t BigStep = 100;
		static constexpr int32_t StepWarning = 100;
	private:
		SelectionShortcuts m_SelectionShortcuts;
		VersionShortcutManager m_VersionManager;

		ExecutionWidget m_ExecutionWidget;
		ResizeWidget m_ResizeWidget;
		StepWidget m_StepWidget;
		DelayWidget m_DelayWidget;
		EditorWidget m_EditorWidget;
	};
}

#endif