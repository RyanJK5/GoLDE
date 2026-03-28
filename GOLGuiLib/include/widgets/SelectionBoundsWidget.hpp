#ifndef SelectionBoundsWidget_hpp_
#define SelectionBoundsWidget_hpp_

#include <imgui.h>
#include <span>

#include "Widget.hpp"

namespace gol {
class SelectionBoundsWidget : public Widget {
  public:
    friend Widget;

  private:
    WidgetResult UpdateImpl(const EditorResult& state);
    void SetShortcutsImpl(const ShortcutMap&) {}

  private:
    Rect m_Bounds;
};
} // namespace gol

#endif