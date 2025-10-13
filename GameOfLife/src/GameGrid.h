#ifndef __GameGrid_h__
#define __GameGrid_h__

#include <vector>

#include "Graphics2D.h"

namespace gol
{
	class GameGrid
	{
	public:
		GameGrid(const std::vector<unsigned char>& seedBuffer, uint32_t width, uint32_t height);

		GameGrid(uint32_t width, uint32_t height);

		void Update();

		inline uint32_t Width() const { return m_width; }
		inline uint32_t Height() const { return m_height; }

		bool Dead() const;

		inline bool Enable(uint32_t x, uint32_t y) { return Set(x, y, true); }
		inline bool Disable(uint32_t x, uint32_t y) { return Set(x, y, false); }
		bool Set(uint32_t x, uint32_t y, bool active);
		bool Toggle(uint32_t x, uint32_t y);

		inline bool Get(uint32_t x, uint32_t y) const { return m_grid[y * m_width + x]; }

		Vec2F GLCoords(uint32_t x, uint32_t y) const;

		Vec2F GLCellDimensions() const;

		std::vector<float> GenerateGLBuffer() const;
	private:
		void ParseSeed(const std::vector<unsigned char>& seedBuffer);

		uint8_t CountNeighbors(uint32_t x, uint32_t y);
	private:
		std::vector<bool> m_grid;
		uint32_t m_width;
		uint32_t m_height;
	};
}

#endif