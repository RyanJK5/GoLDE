#ifndef __Logger_h__
#define __Logger_h__

#include <format>
#include <source_location>
#include <print>

#ifdef _DEBUG
	#define GL_DEBUG(statement)\
		while (glGetError());\
		statement;\
		gol::LogGLErrors(#statement)
	#define ERROR(str, ...) gol::Log(gol::LogCode::Error, std::source_location::current(), str, __VA_ARGS__)
	#define WARNING(str, ...) gol::Log(gol::LogCode::Error, std::source_location::current(), str, __VA_ARGS__)
	#define INFO(str, ...) gol::Log(gol::LogCode::Info, std::source_location::current(), str, __VA_ARGS__)
#else
	#define GL_DEBUG(statement) statement
	#define ERROR(str, ...)
	#define WARNING(str, ...)
	#define INFO(str, ...)
#endif

namespace gol 
{
	enum class LogCode
	{
		GLError,
		Error,
		Warning,
		Info
	};

	inline LogCode Level = LogCode::Info;

	namespace logimpl
	{
		constexpr std::string_view StringRepresentation(gol::LogCode code);

		std::string SimplifyFileName(const std::string& fileName);

		std::string SimplifyFunctionName(const std::string& funcName);
	}

	template <typename... Args>
	inline void Log(LogCode code, const std::source_location& location, std::format_string<Args...> str = "", Args&&... args)
	{
		if (code < Level)
			return;
		
		std::println("{} at {}:{} in {}",
			logimpl::StringRepresentation(code),
			logimpl::SimplifyFileName(location.file_name()),
			location.line(),
			logimpl::SimplifyFunctionName(location.function_name())
		);

		if (!str.get().empty())
			std::println(str, std::forward<Args>(args)...);
	}

	void LogGLErrors(const std::string& call, const std::source_location& location = std::source_location::current());
}

#endif