#ifndef __Game_h__
#define __Game_h__

#include <cstdint>
#include <filesystem>
#include <imgui/imgui.h>
#include <vector>

#include "GameEnums.h"
#include "Graphics2D.h"
#include "PopupWindow.h"
#include "PresetSelection.h"
#include "SimulationControl.h"
#include "SimulationEditor.h"

namespace gol {
class OpenGLWindow {
public:
  Rect Bounds;

  OpenGLWindow(int32_t width, int32_t height);
  ~OpenGLWindow();

  OpenGLWindow(const OpenGLWindow &other) = delete;
  OpenGLWindow(OpenGLWindow &&other) noexcept = delete;

  OpenGLWindow &operator=(const OpenGLWindow &other) = delete;
  OpenGLWindow &operator=(OpenGLWindow &&other) noexcept = delete;

  GLFWwindow *Get() const { return Underlying; }

private:
  GLFWwindow *Underlying;
};

class Game {
public:
  static constexpr int32_t DefaultWindowWidth = 1920;
  static constexpr int32_t DefaultWindowHeight = 1080;
  static constexpr int32_t DefaultGridWidth = 0;
  static constexpr int32_t DefaultGridHeight = 0;

public:
  Game(const std::filesystem::path &configPath);
  ~Game();

  Game(const Game &other) = delete;
  Game(const Game &&other) = delete;
  Game &operator=(const Game &other) = delete;
  Game &operator=(const Game &&other) = delete;

  void Begin();

  void UpdateEditors(SimulationControlResult &controlResult,
                     const PresetSelectionResult &presetResult);

  static inline bool GetKeyState(ImGuiKey keyCode) {
    return ImGui::IsKeyDown(keyCode);
  }
  static inline bool GetMouseState(int32_t mouseButtonCode) {
    return ImGui::IsMouseDown(mouseButtonCode);
  }
  static inline Vec2F CursorPos() { return ImGui::GetMousePos(); }

  inline bool Open() const { return !glfwWindowShouldClose(m_Window.Get()); }

private:
  void HandleUnsaved(PopupWindowState state);

  void BeginFrame();
  void EndFrame();

  void InitImGUI(const std::filesystem::path &stylePath);
  void CreateDockspace();

  bool WindowCanClose();
  void HandleWindowClose(PopupWindowState state);

  bool CheckForNewEditors(const SimulationControlResult &controlResult);
  void CreateEditorDockspace();

  void InitDockspace(uint32_t dockspaceID, ImVec2 windowSize);

private:
  EditorResult m_State = {.State = SimulationState::Paint};
  size_t m_LastActive = 0;

  OpenGLWindow m_Window;

  std::vector<SimulationEditor> m_Editors;
  uint32_t m_EditorCounter = 0;

  WarnWindow m_UnsavedWarning;
  SimulationEditor *m_Unsaved = nullptr;

  size_t m_ActiveEditorID = 0;

  SimulationControl m_Control;
  PresetSelection m_PresetSelection;
  ImFont *m_Font = nullptr;

  bool m_Startup = true;
};
} // namespace gol

#endif