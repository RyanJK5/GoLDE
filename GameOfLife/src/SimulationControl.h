#ifndef __SimulationControl_h__
#define __SimulationControl_h__

#include <memory>
#include <vector>

#include "GameEnums.h"
#include "EditorWidget.h"
#include "GUILoader.h"
#include "StepWidget.h"
#include "ResizeWidget.h"
#include "ExecutionWidget.h"
#include "DelayWidget.h"

namespace gol
{
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
		ExecutionWidget m_ExecutionWidget;
		ResizeWidget m_ResizeWidget;
		StepWidget m_StepWidget;
		DelayWidget m_DelayWidget;
		EditorWidget m_EditorWidget;
	};
}

#endif