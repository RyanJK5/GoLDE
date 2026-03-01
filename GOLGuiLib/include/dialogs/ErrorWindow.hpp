#ifndef ErrorWindow_h_
#define ErrorWindow_h_

#include <optional>
#include <string_view>

#include "PopupWindow.hpp"

namespace gol {
class ErrorWindow : public PopupWindow {
  public:
    ErrorWindow(std::string_view title,
                std::function<void(PopupWindowState)> onUpdate)
        : PopupWindow(title, onUpdate) {}

  protected:
    virtual std::optional<PopupWindowState> ShowButtons() const override final;
};
} // namespace gol

#endif