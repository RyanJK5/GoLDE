#ifndef __StyleLoader_h__
#define __StyleLoader_h__

#include <iostream>
#include <unordered_map>
#include <filesystem>
#include <cctype>
#include <variant>
#include <fstream>
#include <expected>
#include <exception>
#include <concepts>

namespace gol::StyleLoader
{
	template <typename T>
	concept Vector4 =
		std::floating_point<decltype(T::x)> &&
		std::floating_point<decltype(T::y)> &&
		std::floating_point<decltype(T::z)> &&
		std::floating_point<decltype(T::w)> &&
		std::default_initializable<T>;

	static_assert(Vector4<ImVec4>);

	enum class StyleColor
	{
		Transparent, Background, Contrast, Hover, Text
	};

	enum class SectionType
	{
		None, StyleColors, ImGUIStyle, Shortcuts
	};

	enum class YAMLErrorType
	{
		FileOpenError, ParseError, InvalidArguments
	};

	struct YAMLError
	{
		YAMLErrorType Type;
		std::string Description;
	};

	class StyleLoaderException : public std::exception
	{
	public:
		StyleLoaderException() : std::exception() {}
		StyleLoaderException(std::string_view str) : std::exception(std::move(str).data()) {}
		StyleLoaderException(const YAMLError& err) : std::exception(err.Description.c_str()) {}
	};

	template <Vector4 Vec>
	struct StyleInfo
	{
		std::unordered_map<StyleColor, Vec> StyleColors;
		std::unordered_map<ImGuiCol_, StyleColor> AttributeColors;
		std::unordered_map<GameAction, std::vector<ImGuiKeyChord>> Shortcuts;
	};

	static const std::unordered_map<std::string_view, StyleColor> ColorDefinitions = {
		{ "transparent", StyleColor::Transparent },
		{ "background", StyleColor::Background },
		{ "contrast", StyleColor::Contrast },
		{ "hover", StyleColor::Hover },
		{ "text", StyleColor::Text }
	};

	static const std::unordered_map<std::string_view, ImGuiCol_> AttributeDefinitions = {
		{ "ImGuiCol_WindowBg", ImGuiCol_WindowBg },
		{ "ImGuiCol_Border", ImGuiCol_Border },
		{ "ImGuiCol_Text", ImGuiCol_Text },
		{ "ImGuiCol_Button", ImGuiCol_Button },
		{ "ImGuiCol_ButtonHovered", ImGuiCol_ButtonHovered },
		{ "ImGuiCol_Header", ImGuiCol_Header },
		{ "ImGuiCol_HeaderActive", ImGuiCol_HeaderActive },
		{ "ImGuiCol_TitleBg", ImGuiCol_TitleBg },
		{ "ImGuiCol_TitleBgActive", ImGuiCol_TitleBgActive },
		{ "ImGuiCol_Tab", ImGuiCol_Tab },
		{ "ImGuiCol_TabSelectedOverline", ImGuiCol_TabSelectedOverline },
		{ "ImGuiCol_TabDimmedSelected", ImGuiCol_TabDimmedSelected },
		{ "ImGuiCol_TabHovered", ImGuiCol_TabHovered },
		{ "ImGuiCol_TabUnfocused", ImGuiCol_TabUnfocused },
		{ "ImGuiCol_TabDimmed", ImGuiCol_TabDimmed },
		{ "ImGuiCol_TabSelected", ImGuiCol_TabSelected },
	};

	static const std::unordered_map<std::string_view, SectionType> SectionDefinitions = {
		{ "colors", SectionType::StyleColors },
		{ "imgui-style", SectionType::ImGUIStyle },
		{ "shortcuts", SectionType::Shortcuts }
	};

	static const std::unordered_map<std::string_view, ImGuiKey> ShortcutDefinitions = {
		{ "Ctrl", ImGuiMod_Ctrl },
		{ "Enter", ImGuiKey_Enter },
		{ "Space", ImGuiKey_Space },
		{ "R", ImGuiKey_R }
	};

	static const std::unordered_map<std::string_view, GameAction> ActionDefinitions = {
		{ "start", GameAction::Start },
		{ "pause", GameAction::Pause },
		{ "resume", GameAction::Resume },
		{ "restart", GameAction::Restart },
		{ "reset", GameAction::Reset },
		{ "clear", GameAction::Clear }
	};


	namespace {
		template <typename T>
		using ConversionFunction = std::function<std::optional<T>(std::string_view)>;

		template <typename T>
		ConversionFunction<T> GenerateConversion(
			std::unordered_map<std::string_view, T> map)
		{
			return ConversionFunction<T> { [map] (std::string_view str)
			{
				if (map.count(str) == 0)
					return std::optional<T>(std::nullopt);
				return std::make_optional<T>(map.at(str));
			} };
		}

		ConversionFunction<ImGuiKeyChord> GenerateChordConverter(
			std::unordered_map<std::string_view, ImGuiKey> keyMap)
		{
			return ConversionFunction<ImGuiKeyChord> { [keyMap](std::string_view str)
			{
				std::optional<ImGuiKeyChord> chord;
				std::string token = "";
				for (char c : str)
				{
					if (c == '+')
					{
						if (keyMap.count(token) == 0)
							return std::optional<ImGuiKeyChord>(std::nullopt);
						
						chord = !chord ? keyMap.at(token) : (*chord | keyMap.at(token));
						token = "";
						continue;
					}
					token += c;
				}
				if (keyMap.count(token) == 0)
					return std::optional<ImGuiKeyChord>(std::nullopt);
				chord = !chord ? keyMap.at(token) : (*chord | keyMap.at(token));
				return chord;
			}};
		}

		template <typename T>
		std::expected<T, YAMLError> ReadKey(
			int lineNum,
			const std::string& line,
			const ConversionFunction<T>& conversion)
		{
			auto firstLetter = std::find_if(line.begin(), line.end(), [](char c) { return std::isalpha(c); });
			auto seperator = std::find(firstLetter, line.end(), ':');

			if (seperator == line.end())
			{
				return std::unexpected(YAMLError{
					YAMLErrorType::ParseError,
					std::format("Could not find ':' in line {}:\n    {}", lineNum, line)
					});
			}

			auto keyStr = std::string(firstLetter, seperator);
			auto key = conversion(keyStr);
			if (!key)
			{
				return std::unexpected(YAMLError{
					YAMLErrorType::ParseError,
					std::format("'{}' is not a valid key in line {}:\n    {}", keyStr, lineNum, line)
					});
			}
			return *key;
		}

		template <typename T>
		std::expected<std::vector<T>, YAMLError> ReadList(
			int lineNum, const std::string& line, std::string_view values,
			const ConversionFunction<T>& conversion, int numElements = -1)
		{
			std::vector<T> result = {};
			if (numElements >= 0)
				result.reserve(numElements);

			std::string token = "";
			int index = 0;
			for (char c : values)
			{
				if (c == '[' || std::isspace(c))
					continue;
				if (c == ',' || c == ']')
				{
					auto converted = conversion(token);
					if (!converted)
					{
						return std::unexpected(YAMLError{
							YAMLErrorType::InvalidArguments,
							std::format("{} is not a valid value in line {}:\n    {}", token, lineNum, line)
							});
					}
					result.push_back(*converted);
					index++;
					token = "";
					continue;
				}
				token += c;
			}

			if (numElements >= 0 && index != numElements)
			{
				return std::unexpected(YAMLError{
					YAMLErrorType::InvalidArguments,
					std::format("Expected {} parameters, received {} in line {}:\n    {}", numElements, index, lineNum, line)
					});
			}

			return result;
		}

		template <Vector4 Vec>
		Vec CreateVector4(const std::vector<float>& elements)
		{
			auto result = Vec{};
			for (size_t i = 0; i < elements.size(); i++)
			{
				switch (i)
				{
				case 0: result.x = elements[i]; break;
				case 1: result.y = elements[i]; break;
				case 2: result.z = elements[i]; break;
				case 3: result.w = elements[i]; break;
				}
			}
			return result;
		}

		template <typename Key, typename Value>
		std::expected<std::pair<Key, std::vector<Value>>, YAMLError> ReadListPair(
			int lineNum,
			const std::string& line,
			const std::string::const_iterator& firstLetter,
			const ConversionFunction<Key>& keyConversion,
			const ConversionFunction<Value>& valueConversion,
			int numElements = -1
		)
		{
			auto seperator = std::find(firstLetter, line.end(), ':');

			auto key = ReadKey<Key>(lineNum, line, keyConversion);
			if (!key)
				return std::unexpected(key.error());

			auto values = std::string_view(seperator + 1, line.end());
			auto valueList = ReadList<Value>(lineNum, line, values, valueConversion, numElements);
			if (!valueList)
				return std::unexpected(valueList.error());

			return std::pair<Key, std::vector<Value>> {*key, * valueList};
		}

		template <Vector4 Vec>
		std::expected<std::pair<StyleColor, Vec>, YAMLError> ReadColorPair(
			int lineNum,
			const std::string& line,
			const std::string::const_iterator& firstLetter)
		{
			ConversionFunction<float> toF = [](auto str) { return std::make_optional<float>(std::atof(str.data())); };
			ConversionFunction<StyleColor> toC = GenerateConversion(ColorDefinitions);
			auto pair = ReadListPair<StyleColor, float>(lineNum, line, firstLetter,
				toC, toF, 4);
			if (!pair)
				return std::unexpected(pair.error());
			return std::pair<StyleColor, Vec> { pair->first, CreateVector4<Vec>(pair->second) };
		}

		template <typename Key, typename Value>
		std::expected<std::pair<Key, Value>, YAMLError> ReadPair(
			int lineNum,
			const std::string& line,
			const std::string::const_iterator& firstLetter,
			const std::unordered_map<std::string_view, Key>& keyDefinitions,
			const std::unordered_map<std::string_view, Value>& valueDefinitions)
		{
			auto seperator = std::find(firstLetter, line.end(), ':');

			auto key = ReadKey<Key>(lineNum, line, GenerateConversion(keyDefinitions));
			if (!key)
				return std::unexpected(key.error());

			std::string valueStr = "";
			for (auto it = seperator + 1; it != line.end(); it++)
			{
				if (!std::isspace(*it))
					valueStr += *it;
			}

			if (valueDefinitions.count(valueStr) == 0)
			{
				return std::unexpected(YAMLError{
					YAMLErrorType::ParseError,
					std::format("'{}' is not a valid color in line {}:\n    {}", valueStr, lineNum, line)
					});
			}

			return std::pair<Key, Value> { *key, valueDefinitions.at(valueStr) };
		}
	}

	template <Vector4 Vec>
	std::expected<StyleInfo<Vec>, YAMLError> LoadStyle(
		const std::filesystem::path& styleInfoPath)
	{
		std::ifstream input(styleInfoPath);
		if (!input.is_open())
		{
			return std::unexpected(YAMLError{
				YAMLErrorType::FileOpenError,
				std::format("Could not open file '{}'", styleInfoPath.generic_string())
				});
		}

		std::string line = "";
		int indentWidth = 0;
		int lineNum = 0;

		auto section = SectionType::None;
		auto output = StyleInfo<Vec> { };

		while (std::getline(input, line))
		{
			lineNum++;


			auto start = std::find_if(line.begin(), line.end(), [](char c) { return std::isalpha(c); });
			if (start == line.end())
				continue;
			if (std::find(line.begin(), line.end(), '#') < start)
				continue;

			if (indentWidth == 0)
				indentWidth = std::distance(line.begin(), start);
			int depth = indentWidth != 0 ? (std::distance(line.begin(), start) / indentWidth) : 0;

			if (depth == 0)
				section = SectionType::None;

			switch (section)
			{
			case SectionType::StyleColors:
			{
				auto result = ReadColorPair<Vec>(lineNum, line, start);
				if (!result)
					return std::unexpected<YAMLError>(result.error());
				output.StyleColors[result->first] = result->second;
			}	break;
			case SectionType::ImGUIStyle:
			{
				auto result = ReadPair<ImGuiCol_, StyleColor>(
					lineNum, line, start, AttributeDefinitions, ColorDefinitions);
				if (!result)
					return std::unexpected<YAMLError>(result.error());
				output.AttributeColors[result->first] = result->second;
			}	break;
			case SectionType::Shortcuts:
			{
				auto result = ReadListPair<GameAction, ImGuiKeyChord>(
					lineNum, line, start, 
					GenerateConversion(ActionDefinitions), GenerateChordConverter(ShortcutDefinitions));
				if (!result)
					return std::unexpected<YAMLError>(result.error());

				output.Shortcuts[result->first] = result->second;
			}	break;
			}

			auto end = std::find_if(line.rbegin(), line.rend(), [](char c) { return !std::isspace(c); });
			if (*end == ':')
			{
				auto sectionHeader = std::string_view(start, --end.base());
				if (SectionDefinitions.count(sectionHeader) == 0)
				{
					return std::unexpected(YAMLError{
						YAMLErrorType::InvalidArguments,
						std::format("'{}' is not a valid section type in line {}:\n    {}", sectionHeader, lineNum, line)
						});
				}
				
				section = SectionDefinitions.at(sectionHeader);
			}
		}

		return output;
	}
}

#endif