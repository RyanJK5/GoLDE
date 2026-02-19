#ifndef __InputString_h__
#define __InputString_h__

#include <cstring>
#include <print>

namespace gol
{
	class InputString
	{
	public:
		size_t Length = 0;
		char* Data = nullptr;
	public:
		InputString() = default;
		InputString(size_t length);

		template <size_t N>
		InputString(const char(&str)[N], size_t length = N - 1)
			: Length(length)
			, Data(new char[length + 1])
		{
			std::println("{}", length);
			std::memcpy(Data, str, N);
			for (auto i = N; i <= length; i++)
				Data[i] = '\0';
		}

		InputString(const InputString& other);
		InputString(InputString&& other) noexcept;
		InputString& operator=(const InputString& other);
		InputString& operator=(InputString&& other) noexcept;
		~InputString();
	private:
		void Copy(const InputString& other);
		void Move(InputString&& other);
		void Destroy();
	};
}

#endif