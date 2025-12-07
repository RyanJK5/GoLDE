#ifndef __SimulationEditor_h__
#define __SimulationEditor_h__

#include <glm/glm.hpp>
#include <optional>

#include "ErrorWindow.h"
#include "GameEnums.h"
#include "GameGrid.h"
#include "GraphicsHandler.h"
#include "Graphics2D.h"
#include "SelectionManager.h"
#include "SimulationControlResult.h"
#include "VersionManager.h"
#include "WarnWindow.h"

namespace gol
{
	class SimulationEditor
	{
	public:
		static constexpr float DefaultCellWidth = 20.f;
		static constexpr float DefaultCellHeight = 20.f;
	public:
		SimulationEditor(Size2 windowSize, Size2 gridSize);

		Rect WindowBounds() const;
		Rect ViewportBounds() const;

		EditorState Update(const SimulationControlResult& args);
	private:
		SimulationState SimulationUpdate(const GraphicsHandlerArgs& args);
		SimulationState PaintUpdate(const GraphicsHandlerArgs& args);
		SimulationState PauseUpdate(const GraphicsHandlerArgs& args);

		void UpdateVersion(const SimulationControlResult& args);

		void DisplaySimulation();

		SimulationState UpdateState(const SimulationControlResult& action);
		void PasteSelection();
		void LoadFile(const gol::SimulationControlResult& result);
		gol::SimulationState ResizeGrid(const gol::SimulationControlResult& result);
		void UpdateViewport();
		std::optional<Vec2> CursorGridPos();
		std::optional<Vec2> ConvertToGridPos(Vec2F screenPos);

		void UpdateMouseState(Vec2 gridPos);
		void FillCells();
		void UpdateDragState();

	private:
		static constexpr double DefaultTickDelayMs = 0.;
	private:
		GameGrid m_Grid;
		GameGrid m_InitialGrid;

		SelectionManager m_SelectionManager;
		VersionManager m_VersionManager;

		GraphicsHandler m_Graphics;
		RectF m_WindowBounds;

		ErrorWindow m_FileErrorWindow;
		WarnWindow m_PasteWarning;

		bool m_TakeMouseInput = false;
		Vec2F m_LeftDeltaLast;
		Vec2F m_RightDeltaLast;
		double m_TickDelayMs = DefaultTickDelayMs;
		EditorMode m_EditorMode = EditorMode::None;
	};
}

#endif