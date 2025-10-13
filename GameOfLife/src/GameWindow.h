#ifndef __GLWindow_h__
#define __GLWindow_h__

#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "ShaderManager.h"
#include "Graphics2D.h"

template <>
struct std::default_delete<GLFWwindow>
{
	void operator() (GLFWwindow*) { glfwTerminate(); }
};

namespace gol
{
	class GameWindow
	{
	public:
		GameWindow(int32_t width, int32_t height);
		GameWindow(Size2 size);

		~GameWindow();

		GameWindow(const GameWindow& other) = delete;
		GameWindow(GameWindow&& other) = delete;
		GameWindow& operator=(const GameWindow& other) = delete;
		GameWindow& operator=(GameWindow&& other) = delete;

		Rect WindowBounds() const;
		Rect ViewportBounds(Size2 gridSize) const;

		inline bool GetKeyState(ImGuiKey keyCode) const { return ImGui::IsKeyDown(keyCode); }
		inline bool GetMouseState(int32_t mouseButtonCode) const { return ImGui::IsMouseDown(mouseButtonCode); }
		Vec2F CursorPos() const;

		inline bool Open() const { return !glfwWindowShouldClose(m_Window.get()); }

		void FrameStart(uint32_t textureID);
		void FrameEnd() const;

		void UpdateViewport(Size2 gridSize) const;
	private:
		static constexpr int32_t GOLImGuiConfig = 
			ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
	private:
		void InitImGUI();
	private:
		std::unique_ptr<GLFWwindow> m_Window;
		RectF m_WindowBounds;
	};
}
#endif