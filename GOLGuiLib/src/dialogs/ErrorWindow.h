#ifndef __ErrorWindow_h__
#define __ErrorWindow_h__

#include <string_view>

#include "PopupWindow.h"

namespace gol
{
	class ErrorWindow : public PopupWindow
	{
	public:
		ErrorWindow(std::string_view title, std::string_view message = "")
			: PopupWindow(title, message)
		{ }
	protected:
		virtual PopupWindowState ShowButtons() const override final;
	};
}

#endif