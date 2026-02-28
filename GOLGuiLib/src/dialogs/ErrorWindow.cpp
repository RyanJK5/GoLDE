#include <cstdint>
#include <imgui/imgui.h>

#include "ActionButton.h"
#include "ErrorWindow.h"
#include "GameEnums.h"
#include "PopupWindow.h"

gol::PopupWindowState gol::ErrorWindow::ShowButtons() const {
  constexpr static int32_t height =
      ActionButton<EditorAction, false>::DefaultButtonHeight;
  bool ok = ImGui::Button("Ok", {ImGui::GetContentRegionAvail().x, height});

  return ok ? PopupWindowState::Success : PopupWindowState::None;
}