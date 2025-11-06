#ifndef __SimulationEditor_h__
#define __SimulationEditor_h__

#include <vector>
#include <optional>
#include <functional>
#include <unordered_map>

#include "SimulationControl.h"
#include "GameGrid.h"
#include "GraphicsHandler.h"
#include "Graphics2D.h"
#include "GameActionButton.h"

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

		GameState Update(const SimulationControlResult& args);
	private:
		GameState SimulationUpdate(const GraphicsHandlerArgs& args);
		GameState PaintUpdate(const GraphicsHandlerArgs& args);
		GameState PauseUpdate(const GraphicsHandlerArgs& args);

		void DisplaySimulation();

		GameState UpdateState(const SimulationControlResult& action);
		void UpdateViewport();
		std::optional<Vec2> CursorGridPos();
		void UpdateMouseState(Vec2 gridPos);
		void UpdateDragState();
	private:
		static constexpr double DefaultTickDelayMs = 10.;
	private:
		GameGrid m_Grid;
		GameGrid m_InitialGrid;
		GraphicsHandler m_Graphics;
		
		std::vector<GameActionButton> m_Buttons;
		RectF m_WindowBounds;

		glm::vec2 m_DeltaLast = { 0, 0 };
		double m_TickDelayMs = DefaultTickDelayMs;
		DrawMode m_DrawMode = DrawMode::None;
	};
}

#endif