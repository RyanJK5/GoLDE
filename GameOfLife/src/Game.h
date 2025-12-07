#ifndef __Game_h__
#define __Game_h__

#include <cstdint>
#include <filesystem>
#include <imgui/imgui.h>

#include "GameEnums.h"
#include "Graphics2D.h"
#include "PresetSelection.h"
#include "SimulationControl.h"
#include "SimulationEditor.h"

template <>
struct std::default_delete<GLFWwindow>
{
	void operator() (GLFWwindow*) { glfwTerminate(); }
};

namespace gol
{
	class OpenGLWindow
	{
	public:
		OpenGLWindow(int32_t width, int32_t height);
		~OpenGLWindow();

		Rect Bounds;
		inline GLFWwindow* Get() const { return Underlying; }
	private:
		GLFWwindow* Underlying;
	};

	class Game
	{
	public:
		static constexpr int32_t DefaultWindowWidth = 1920;
		static constexpr int32_t DefaultWindowHeight = 1080;
		static constexpr int32_t DefaultGridWidth = 0;
		static constexpr int32_t DefaultGridHeight = 0;
	public:
		Game();
		~Game();

		Game(const Game& other) = delete;
		Game(const Game&& other) = delete;
		Game& operator=(const Game& other) = delete;
		Game& operator=(const Game&& other) = delete;
	
		void Begin();

		static inline bool GetKeyState(ImGuiKey keyCode) { return ImGui::IsKeyDown(keyCode); }
		static inline bool GetMouseState(int32_t mouseButtonCode) { return ImGui::IsMouseDown(mouseButtonCode); }
		static inline Vec2F CursorPos() { return ImGui::GetMousePos(); }

		inline bool Open() const { return !glfwWindowShouldClose(m_Window.Get()); }
	private:
		void BeginFrame();
		void EndFrame();

		void InitImGUI(const std::filesystem::path& stylePath);
		void CreateDockspace();
		void InitDockspace(uint32_t dockspaceID, ImVec2 windowSize);

	private:
		EditorState m_State = { .State = SimulationState::Paint };

		OpenGLWindow m_Window;
		SimulationEditor m_Editor;
		SimulationControl m_Control;
		PresetSelection m_PresetSelection;
		ImFont* m_Font = nullptr;

		bool m_Startup = true;
	};
}

#endif