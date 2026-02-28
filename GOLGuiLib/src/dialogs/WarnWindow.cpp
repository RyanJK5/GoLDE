#include <cstdint>
#include <imgui/imgui.h>
#include <string>

#include "ActionButton.h"
#include "GameEnums.h"
#include "PopupWindow.h"
#include "WarnWindow.h"

namespace gol
{

std::optional<PopupWindowState> WarnWindow::ShowButtons() const {
  constexpr int32_t height =
      ActionButton<EditorAction, false>::DefaultButtonHeight;
  bool yes = ImGui::Button("Yes", {ImGui::GetContentRegionAvail().x, height});
  bool no = ImGui::Button("No", {ImGui::GetContentRegionAvail().x, height});

  if (yes)
    return PopupWindowState::Success;
  if (no)
    return PopupWindowState::Failure;
  return {};
}

}