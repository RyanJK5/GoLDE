#ifndef __GameGrid_h__
#define __GameGrid_h__

#include <vector>
#include <optional>

#include "Graphics2D.h"

namespace gol
{
	class GameGrid
	{
	public:
		GameGrid(const std::vector<unsigned char>& seedBuffer, int32_t width, int32_t height);

		GameGrid(int32_t width, int32_t height);
		GameGrid(Size2 size);

		void Update();

		inline int32_t Width() const { return m_Width; }
		inline int32_t Height() const { return m_Height; }
		inline Size2 Size() const { return { m_Width, m_Height }; }

		bool Dead() const;

		inline bool Enable(int32_t x, int32_t y) { return Set(x, y, true); }
		inline bool Disable(int32_t x, int32_t y) { return Set(x, y, false); }
		bool Set(int32_t x, int32_t y, bool active);
		bool Toggle(int32_t x, int32_t y);
		std::optional<bool> Get(int32_t x, int32_t y) const;

		Vec2F GLCoords(int32_t x, int32_t y) const;

		Size2F GLCellDimensions() const;

		std::vector<float> GenerateGLBuffer() const;
	private:
		void ParseSeed(const std::vector<unsigned char>& seedBuffer);

		int8_t CountNeighbors(int32_t x, int32_t y);
	private:
		std::vector<bool> m_Grid;
		int32_t m_Width;
		int32_t m_Height;
	};
}

#endif