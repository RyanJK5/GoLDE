#ifndef __PopupWindow_h__
#define __PopupWindow_h__

#include <functional>
#include <optional>
#include <string>
#include <string_view>

namespace gol {
enum class PopupWindowState { Success, Failure };

class PopupWindow {
public:
  PopupWindow(std::string_view title,
              std::function<void(PopupWindowState)> onUpdate);

  void Update();

  void Activate() { Active = true; }

  void SetCallback(std::function<void(PopupWindowState)> onUpdate);

  std::string Message;

protected:
  virtual std::optional<PopupWindowState> ShowButtons() const = 0;

private:
  std::function<void(PopupWindowState)> m_UpdateCallback;

  bool Active = false;
  std::string m_Title;
};
} // namespace gol

#endif