#ifndef __Game_h__
#define __Game_h__

#include <optional>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "GameGrid.h"
#include "DrawHandler.h"

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
		~Game();
	
		void Begin();
	private:
		void InitOpenGL();
		void InitImGUI();

		void UpdateState();
		void UpdateViewport();

		Rect WindowBounds() const;
		Rect ViewportBounds() const;

		std::optional<Vec2> GetCursorGridPos();
		void UpdateMouseState(Vec2 gridPos);

		bool SimulationUpdate(double timeElapsedMs);
		void PaintUpdate();

	private:
		static struct InputState
		{
			bool EnterDown;
			DrawMode DrawMode;

			InputState()
				: EnterDown(false)
				, DrawMode(DrawMode::None)
			{ }
		};

		static struct SimulationSettings
		{
			GameState State;
			double TickDelayMs;

			SimulationSettings(GameState state, double tickDelayMs = 0)
				: State(state)
				, TickDelayMs(tickDelayMs)
			{}
		};
	private:
		GLFWwindow* m_window;
		DrawHandler m_draw;

		GameGrid m_grid;
		SimulationSettings m_settings;
		InputState m_input;
	};
}

#endif