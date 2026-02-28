#ifndef __WarnWindow_h__
#define __WarnWindow_h__

#include <string>
#include <string_view>

#include "PopupWindow.h"

namespace gol {
class WarnWindow : public PopupWindow {
public:
  WarnWindow(std::string_view title,
             std::function<void(PopupWindowState)> onUpdate)
      : PopupWindow(title, onUpdate) {}

protected:
  virtual PopupWindowState ShowButtons() const override final;
};
} // namespace gol

#endif