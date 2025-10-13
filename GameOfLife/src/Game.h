#ifndef __Game_h__
#define __Game_h__

#include <optional>
#include <memory>

#include "GameWindow.h"
#include "GameGrid.h"
#include "GraphicsHandler.h"

namespace gol
{
	enum class GameState
	{
		Paint, Simulation
	};

	enum class DrawMode
	{
		None, Insert, Delete
	};


	class Game
	{
	public:
		static constexpr uint32_t DefaultWindowWidth = 1920;
		static constexpr uint32_t DefaultWindowHeight = 1080;
		static constexpr uint32_t DefaultGridWidth = DefaultWindowWidth / 15;
		static constexpr uint32_t DefaultGridHeight = DefaultWindowHeight / 15;
	public:
		Game();

		Game(const Game& other) = delete;
		Game(const Game&& other) = delete;
		Game& operator=(const Game& other) = delete;
		Game& operator=(const Game&& other) = delete;
	
		void Begin();
	private:
		void InitImGUI();

		void UpdateState();

		std::optional<Vec2> CursorGridPos();
		void UpdateMouseState(Vec2 gridPos);

		bool SimulationUpdate(double timeElapsedMs);
		void PaintUpdate();

	private:
		struct InputState
		{
			DrawMode DrawMode;
			bool EnterDown;

			InputState()
				: DrawMode(DrawMode::None)
				, EnterDown(false)
			{ }
		};

		struct SimulationSettings
		{
			double TickDelayMs;
			GameState State;

			SimulationSettings(GameState state, double tickDelayMs = 0)
				: TickDelayMs(tickDelayMs)
				, State(state)
			{}
		};
	private:
		GameGrid m_Grid = { 64, 64 };
		SimulationSettings m_Settings = { GameState::Paint, 10 };
		InputState m_Input;

		GameWindow m_Window;
		GraphicsHandler m_Graphics;
	};
}

#endif