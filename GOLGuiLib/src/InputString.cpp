#include <utility>

#include "InputString.h"

namespace gol
{
    InputString::InputString(size_t length)
        : Length(length), Data(new char[length + 1])
    {
        for (size_t i = 0; i <= length; i++)
            Data[i] = '\0';
    }

    InputString::InputString(const InputString& other)
    {
        Copy(other);
    }

    InputString::InputString(InputString&& other) noexcept
    {
        Move(std::move(other));
    }

    InputString& InputString::operator=(const InputString& other)
    {
        if (this != &other)
        {
            Destroy();
            Copy(other);
        }
        return *this;
    }

    InputString& InputString::operator=(InputString&& other) noexcept
    {
        if (this != &other)
        {
            Destroy();
            Move(std::move(other));
        }
        return *this;
    }

    InputString::~InputString()
    {
        Destroy();
    }

    void InputString::Copy(const InputString& other)
    {
        Length = other.Length;
        Data = new char[other.Length + 1];
        for (size_t i = 0; i <= other.Length + 1; i++)
            Data[i] = other.Data[i];
    }

    void InputString::Move(InputString&& other)
    {
        Length = std::exchange(other.Length, 0);
        Data = std::exchange(other.Data, nullptr);
    }

    void InputString::Destroy()
    {
        delete[] Data;
    }
}