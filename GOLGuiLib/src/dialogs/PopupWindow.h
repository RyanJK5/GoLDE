#ifndef __PopupWindow_h__
#define __PopupWindow_h__

#include <string>
#include <string_view>

namespace gol
{
	enum class PopupWindowState
	{
		None,
		Success,
		Failure
	};

	class PopupWindow
	{
	public:
		PopupWindow(std::string_view title, std::string_view message = "");

		PopupWindowState Update();

		bool Active = false;
		std::string Message;
	protected:
		virtual PopupWindowState ShowButtons() const = 0;
	private:
		std::string m_Title;
	};
}

#endif