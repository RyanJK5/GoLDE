#ifndef __GameWindow_h__
#define __GameWindow_h__


#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "GameGrid.h"

namespace gol
{
	class GameWindow
	{
	public:
		GameWindow();

		~GameWindow();

		GameWindow(const GameWindow& other) = delete;

		GameWindow(const GameWindow&& other) = delete;
		
		GameWindow& operator=(const GameWindow& other) = delete;

		GameWindow& operator=(const GameWindow&& other) = delete;
	
		void Begin();

	private:
		std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> ViewportBounds() const;

		void GameLoop();

		void PaintLoop();
		void PaintUpdate();
		
		void DrawGrid() const;

		void DrawSelection(size_t x, size_t y) const;
	private:
		GameGrid m_grid;
		GLFWwindow* m_window;
	};
}

#endif