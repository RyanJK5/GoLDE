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

		PresetDisplay(const GameGrid& grid, const std::string& fileName, Size2 windowSize)
			: Grid(grid), FileName(fileName),
			  Graphics(
				  std::filesystem::path("resources") / "shader" / "default.shader", 
				  windowSize.Width, windowSize.Height
			  )
		{ }
	};

	class PresetSelection
	{
	public:
		static constexpr Size2 TemplateDimensions = { 300, 300 };
		static constexpr int32_t TemplatesPerRow = 5;
	public:
		PresetSelection(const std::filesystem::path& defaultPath, Size2 windowSize);

		PresetSelectionResult Update();
	private:
		void ReadFiles(const std::filesystem::path& path, Size2 windowSize);

		std::vector<PresetDisplay> m_Library;
		Size2F m_MaxGridDimensions;
	};
}

#endif