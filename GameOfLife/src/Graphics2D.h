#ifndef __Graphics2D_h__
#define __Graphics2D_h__

#include <cstdint>
#include <concepts>

namespace gol
{
	template <typename T> requires std::totally_ordered<T>
	struct GenVec
	{
		T X;
		T Y;

		GenVec() : X(0), Y(0) { }
		GenVec(T x, T y) : X(x), Y(y) { }
	};

	template <typename T> requires std::totally_ordered<T>
	struct GenRect
	{
		union
		{
			GenVec<T> Pos;
			struct
			{
				T X;
				T Y;
			};
		};
		union
		{
			GenVec<T> Size;
			struct
			{
				T Width;
				T Height;
			};
		};

		GenRect() : Pos(0,0), Size(0,0) { }
		GenRect(T x, T y, T width, T height) : Pos(x, y), Size(width, height) { }
		GenRect(GenVec<T> pos, GenVec<T> size) : Pos(pos), Size(size) { }

		inline bool InBounds(T x, T y) const { return x > X && x < X + Width && y > Y && y < Y + Height; }
		
		inline bool InBounds(GenVec<T> pos) const { return InBounds(pos.X, pos.Y); }
	};

	using Vec2 = GenVec<int32_t>;
	using Vec2F = GenVec<float>;

	using Rect = GenRect<int32_t>;
	using RectF = GenRect<float>;
}

#endif