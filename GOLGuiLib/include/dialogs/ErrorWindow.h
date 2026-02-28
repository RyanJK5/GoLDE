#ifndef __ErrorWindow_h__
#define __ErrorWindow_h__

#include <string_view>
#include <optional>

#include "PopupWindow.h"

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