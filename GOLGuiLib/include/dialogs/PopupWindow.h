#ifndef __PopupWindow_h__
#define __PopupWindow_h__

#include <functional>
#include <string>
#include <string_view>

namespace gol {
enum class PopupWindowState { None, Success, Failure };

class PopupWindow {
public:
  PopupWindow(std::string_view title,
              std::function<void(PopupWindowState)> onUpdate);

  PopupWindowState Update();

  void Activate() { Active = true; }

  void SetCallback(std::function<void(PopupWindowState)> onUpdate);

  std::string Message;

protected:
  virtual PopupWindowState ShowButtons() const = 0;

private:
  std::function<void(PopupWindowState)> m_UpdateCallback;

  bool Active = false;
  std::string m_Title;
};
} // namespace gol

#endif