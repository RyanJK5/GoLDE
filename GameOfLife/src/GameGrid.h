#ifndef __GameGrid_h__
#define __GameGrid_h__

#include <cstdint>
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

		int32_t Width() const { return m_Width; }
		int32_t Height() const { return m_Height; }
		
		Size2 Size() const { return { m_Width, m_Height }; }
		
		bool Bounded() const { return m_Width > 0 && m_Height > 0; }
		
		Rect BoundingBox() const;
		
		bool InBounds(int32_t x, int32_t y) const { return InBounds({ x, y }); }
		bool InBounds(Vec2 pos) const { return !Bounded() || Rect(0, 0, m_Width, m_Height).InBounds(pos); }

		int64_t Generation() const { return m_Generation; }
		int64_t Population() const { return m_Population; }

		bool Dead() const;

		bool Set(int32_t x, int32_t y, bool active);
		bool Toggle(int32_t x, int32_t y);
		
		void TranslateRegion(const Rect& region, Vec2 translation);

		GameGrid SubRegion(const Rect& region) const;
		
		std::set<Vec2> ReadRegion(const Rect& region) const;
		
		void ClearRegion(const Rect& region);
		
		void ClearData(const std::vector<Vec2>& data, Vec2 pos);
		
		std::set<Vec2> InsertGrid(const GameGrid& grid, Vec2 pos);
		
		void RotateGrid(bool clockwise = true);
		
		void FlipGrid(bool vertical);

		std::optional<bool> Get(int32_t x, int32_t y) const;
		std::optional<bool> Get(Vec2 pos) const;

		constexpr const std::set<Vec2>& Data() const { return m_Data; }
	private:
		std::set<Vec2> m_Data;
		int32_t m_Width;
		int32_t m_Height;

		int64_t m_Population = 0;
		int64_t m_Generation = 0;
	};
}

#endif