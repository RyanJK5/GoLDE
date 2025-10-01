#ifndef __GameGrid_h__
#define __GameGrid_h__
#include <vector>

namespace gol
{
	class GameGrid
	{
	public:
		static constexpr size_t DefaultWidth = 160;
		static constexpr size_t DefaultHeight = 90;
	
		GameGrid(const std::vector<unsigned char>& seedBuffer = {}, size_t width = DefaultWidth, size_t height = DefaultHeight);

		GameGrid(const std::vector<bool>& seed, size_t width = DefaultWidth, size_t height = DefaultHeight);

		void Update();

		inline size_t Width() const { return m_width; }
		inline size_t Height() const { return m_height; }

		bool Dead() const;

		bool Enable(size_t x, size_t y) { return Set(x, y, true); }
		bool Disable(size_t x, size_t y) { return Set(x, y, false); }
		bool Set(size_t x, size_t y, bool active);
		bool Toggle(size_t x, size_t y);

		bool Get(size_t x, size_t y) const { return m_grid[y * m_width + x]; }

		std::pair<float, float> GLCoords(size_t x, size_t y) const;

		std::pair<float, float> GLCellDimensions() const;

		std::vector<float_t> GenerateGLBuffer() const;
	private:
		void ParseSeed(const std::vector<unsigned char>& seedBuffer);

		uint8_t CountNeighbors(size_t x, size_t y);
	private:
		std::vector<bool> m_grid;
		size_t m_width;
		size_t m_height;

	};
}

#endif