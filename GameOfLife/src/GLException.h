#ifndef __GLException_h__
#define __GLException_h__

#include <exception>
#include <string_view>

namespace gol
{
	class GLException : public std::exception
	{
	public:
		GLException() : std::exception() { }

		GLException(std::string_view str) : std::exception(std::move(str).data()) {}
	};
}

#endif