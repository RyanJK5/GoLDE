#ifndef __SimulationEditor_h__
#define __SimulationEditor_h__

#include <vector>
#include <optional>
#include <functional>
#include <unordered_map>

#include "GameGrid.h"
#include "GraphicsHandler.h"
#include "Graphics2D.h"
#include "GameActionButton.h"

namespace gol
{
	struct SimulationEditorArgs
	{
		GameState State;
		GameAction Action;
	};

	class SimulationEditor
	{
	public:
		static constexpr int32_t DefaultCellWidth = 20;
		static constexpr int32_t DefaultCellHeight = 20;
	public:
		SimulationEditor(Size2 windowSize, Size2 gridSize);

		Rect WindowBounds() const;
		Rect ViewportBounds() const;

		GameState Update(const SimulationEditorArgs& args);
	private:
		GameState SimulationUpdate(const GraphicsHandlerArgs& args);
		GameState PaintUpdate(const GraphicsHandlerArgs& args);
		GameState PauseUpdate(const GraphicsHandlerArgs& args);

		void DisplaySimulation();

		GameState UpdateState(GameAction action);
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

		double m_TickDelayMs = DefaultTickDelayMs;
		glm::vec2 m_DeltaLast;
		DrawMode m_DrawMode = DrawMode::None;
	};
}

#endif