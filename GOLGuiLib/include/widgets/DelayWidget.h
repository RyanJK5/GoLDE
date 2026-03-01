#ifndef DelayWidget_h_
#define DelayWidget_h_

#include <cstdint>
#include <imgui/imgui.h>
#include <span>

#include "GameEnums.h"
#include "SimulationControlResult.h"
#include "Widget.h"

namespace gol {
class DelayWidget : public Widget {
  public:
    DelayWidget(std::span<const ImGuiKeyChord> = {}) {}
    friend Widget;

  private:
    SimulationControlResult UpdateImpl(const EditorResult &state);

  private:
    int32_t m_TickDelayMs = 0;
    bool m_GridLines = false;
};
} // namespace gol

#endif