#include <cstdint>
#include <imgui/imgui.h>

#include "ActionButton.h"
#include "ErrorWindow.h"
#include "GameEnums.h"
#include "PopupWindow.h"

namespace gol
{

std::optional<PopupWindowState> ErrorWindow::ShowButtons() const {
  constexpr static int32_t height =
      ActionButton<EditorAction, false>::DefaultButtonHeight;
  const bool ok = ImGui::Button("Ok", {ImGui::GetContentRegionAvail().x, height});

  if (ok)
    return PopupWindowState::Success;
  return {};
}

}