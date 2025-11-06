#ifndef __GameGrid_h__
#define __GameGrid_h__

#include <vector>
#include <set>
#include <optional>

#include "Graphics2D.h"

namespace gol
{
	class GameGrid
	{
	public:
		GameGrid(int32_t width = 0, int32_t height = 0);
		GameGrid(Size2 size);

		GameGrid(const GameGrid& other, Size2 size);

		void Update();

		inline int32_t Width() const { return m_Width; }
		inline int32_t Height() const { return m_Height; }
		inline Size2 Size() const { return { m_Width, m_Height }; }
		inline bool Bounded() const { return m_Width > 0 && m_Height > 0; }
		inline bool InBounds(int32_t x, int32_t y) const { return InBounds({ x, y }); }
		inline bool InBounds(Vec2 pos) const { return !Bounded() || Rect(0, 0, m_Width, m_Height).InBounds(pos); }

		inline int64_t Generation() const { return m_Generation; }
		inline int64_t Population() const { return m_Population; }

		bool Dead() const;

		inline bool Enable(int32_t x, int32_t y) { return Set(x, y, true); }
		inline bool Disable(int32_t x, int32_t y) { return Set(x, y, false); }
		bool Set(int32_t x, int32_t y, bool active);
		bool Toggle(int32_t x, int32_t y);
		std::optional<bool> Get(int32_t x, int32_t y) const;

		const std::set<Vec2>& Data() const { return m_Data; }
	private:
		std::set<Vec2> m_Data;
		int32_t m_Width;
		int32_t m_Height;

		int64_t m_Population = 0;
		int64_t m_Generation = 0;
	};
}

#endif