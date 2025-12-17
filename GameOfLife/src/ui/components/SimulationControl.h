#ifndef __SimulationControl_h__
#define __SimulationControl_h__

#include <cstdint>
#include <imgui/imgui.h>
#include <unordered_map>
#include <vector>

#include "ConfigLoader.h"
#include "DelayWidget.h"
#include "EditorWidget.h"
#include "ExecutionWidget.h"
#include "FileWidget.h"
#include "GameEnums.h"
#include "KeyShortcut.h"
#include "ResizeWidget.h"
#include "SimulationControlResult.h"
#include "StepWidget.h"

namespace gol
{
	struct SelectionShortcuts
	{
		std::unordered_map<SelectionAction, std::vector<KeyShortcut>> Shortcuts;

		SelectionShortcuts(
			const std::vector<ImGuiKeyChord>& left,
			const std::vector<ImGuiKeyChord>& right,
			const std::vector<ImGuiKeyChord>& up,
			const std::vector<ImGuiKeyChord>& down
		);

		SimulationControlResult Update(const EditorResult& state);
	};

	class SimulationControl
	{
	public:
		SimulationControl(const StyleLoader::StyleInfo<ImVec4>& fileInfo);

		SimulationControlResult Update(const EditorResult& state);
	private:
		void FillResults(SimulationControlResult& current, const SimulationControlResult& update) const;
	private:
		static constexpr int32_t BigStep = 100;
		static constexpr int32_t StepWarning = 100;
	private:
		SelectionShortcuts m_SelectionShortcuts;

		ExecutionWidget m_ExecutionWidget;
		EditorWidget m_EditorWidget;
		FileWidget m_FileWidget;
		ResizeWidget m_ResizeWidget;
		StepWidget m_StepWidget;
		DelayWidget m_DelayWidget;
	};
}

#endif