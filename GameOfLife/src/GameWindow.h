#ifndef __GLWindow_h__
#define __GLWindow_h__

#include <memory>
#include <optional>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "GameActionButton.h"
#include "KeyShortcut.h"
#include "ShaderManager.h"
#include "GameEnums.h"
#include "Graphics2D.h"

template <>
struct std::default_delete<GLFWwindow>
{
	void operator() (GLFWwindow*) { glfwTerminate(); }
};

namespace gol
{
	struct UpdateInfo
	{
		GameAction Action;
		std::optional<Vec2> UpdatePosition;
	};

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

		void BeginFrame();
		UpdateInfo CreateGUI(const DrawInfo& inInfo);
		void EndFrame();

		void UpdateViewport(Size2 gridSize);
	private:
		static constexpr int32_t IOFlags = 
			ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;
		static constexpr int32_t DockspaceFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	private:
		void InitImGUI();
		void CreateButtons(const std::unordered_map<GameAction, std::vector<ImGuiKeyChord>>& shortcuts);

		void CreateDockspace();
		void InitDockspace(uint32_t dockspaceID);

		void DisplaySimulation(uint32_t textureID);
		GameAction DisplaySimulationControl(const DrawInfo& info);
	private:
		std::unique_ptr<GLFWwindow> m_Window;
		RectF m_WindowBounds;

		bool m_Startup = true;

		ImFont* m_Font;
		
		std::vector<GameActionButton> m_Buttons;
	};
}
#endif