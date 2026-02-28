#include <string_view>

#include <imgui/imgui.h>

#include "PopupWindow.h"

namespace gol {
PopupWindow::PopupWindow(std::string_view title,
                         std::function<void(PopupWindowState)> onUpdate)
    : Message(""), m_Title(title), m_UpdateCallback(onUpdate) {}

PopupWindowState PopupWindow::Update() {
  if (!Active)
    return PopupWindowState::None;

  ImGui::OpenPopup(m_Title.c_str());
  ImGui::BeginPopupModal(m_Title.c_str(), nullptr,
                         ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoResize);

  ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 30.f);
  ImGui::Text("%s", Message.c_str());
  ImGui::PopStyleVar();

  const auto result = ShowButtons();
  if (result != PopupWindowState::None) {
    m_UpdateCallback(result);
    Active = false;
  }

  ImGui::EndPopup();
  return result;
}

void PopupWindow::SetCallback(std::function<void(PopupWindowState)> onUpdate) {
  m_UpdateCallback = onUpdate;
}

} // namespace gol