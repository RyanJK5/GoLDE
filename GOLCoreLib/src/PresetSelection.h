#ifndef __PresetComponent_h__
#define __PresetComponent_h__

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "GameGrid.h"
#include "Graphics2D.h"
#include "GraphicsHandler.h"
#include "PresetSelectionResult.h"

namespace gol
{
	struct PresetDisplay
	{
		GameGrid Grid;
		std::string FileName;
		GraphicsHandler Graphics;

		PresetDisplay(const GameGrid& grid, const std::string& fileName, Size2 windowSize);
	};

	class SearchString
	{
	public:
		size_t Length = 0;
		char* Data = nullptr;
	public:
		SearchString() = default;
		SearchString(size_t length);
		SearchString(const SearchString& other);
		SearchString(SearchString&& other) noexcept;
		SearchString& operator=(const SearchString& other);
		SearchString& operator=(SearchString&& other) noexcept;
		~SearchString();
	private:
		void Copy(const SearchString& other);
		void Move(SearchString&& other);
		void Destroy();
	};

	class PresetSelection
	{
	public:
		static constexpr Size2 TemplateDimensions = { 300, 300 };
		static constexpr int32_t TemplatesPerRow = 5;
	public:
		PresetSelection(const std::filesystem::path& defaultPath, Size2 templateSize = TemplateDimensions);

		PresetSelectionResult Update();
	private:
		void ReadFiles(const std::filesystem::path& path);

		std::filesystem::path m_DefaultPath;
		Size2 m_WindowSize;

		SearchString m_SearchText;
		size_t m_MaxFileName = 0;

		std::vector<PresetDisplay> m_Library;
		Size2F m_MaxGridDimensions;
	};
}

#endif