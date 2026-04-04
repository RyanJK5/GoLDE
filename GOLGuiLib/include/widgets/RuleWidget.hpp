#ifndef RuleWidget_hpp_
#define RuleWidget_hpp_

#include "ErrorWindow.hpp"
#include "Widget.hpp"

namespace gol {
class RuleWidget : public Widget {
  public:
    RuleWidget();

    friend Widget;

  private:
    WidgetResult UpdateImpl(const EditorResult& state);
    void SetShortcutsImpl(const ShortcutMap&) {}

  private:
    std::string m_InputText = "B3/S23";
    std::string m_LastValid = "B3/S23";
    ErrorWindow m_InputError;
};
} // namespace gol

#endif