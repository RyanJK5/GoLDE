#ifndef __Graphics2D_h__
#define __Graphics2D_h__

#include <cstdint>
#include <concepts>

#include "vendor/imgui.h"

namespace gol
{
	template <typename T> requires std::totally_ordered<T>
	struct GenericVec
	{
		T X;
		T Y;

		GenericVec() : X(0), Y(0) { }
		GenericVec(T x, T y) : X(x), Y(y) { }
	};

	template <typename T> requires std::totally_ordered<T>
	struct GenericSize
	{
		T Width;
		T Height;


		GenericSize() : Width(0), Height(0) { }
		GenericSize(T width, T height) : Width(width), Height(height) { }
	};

	template <typename T> requires std::totally_ordered<T>
	struct GenericRect
	{
		T X;
		T Y;
		T Width;
		T Height;

		inline GenericVec<T> Pos() { return { X, Y }; }
		inline GenericSize<T> Size() { return { Width, Height }; }

		GenericRect(T x, T y, T width, T height) : X(x), Y(y), Width(width), Height(height) { }
		GenericRect(GenericVec<T> pos, GenericSize<T> size) : X(pos.X), Y(pos.Y), Width(size.Width), Height(size.Height) {}

		inline bool InBounds(std::totally_ordered auto x, std::totally_ordered auto y) const
			{ return x > X && x < X + Width && y > Y && y < Y + Height; }
		
		inline bool InBounds(GenericVec<T> pos) const { return InBounds(pos.X, pos.Y); }
	};

	struct Vec2F : public GenericVec<float>
	{
		Vec2F() : GenericVec() { }
		Vec2F(ImVec2 vec) : GenericVec(vec.x, vec.y) { }
		Vec2F(float x, float y) : GenericVec(x, y) { }
	};

	struct Size2F : public GenericSize<float>
	{
		Size2F() : GenericSize() { }
		Size2F(ImVec2 vec) : GenericSize(vec.x, vec.y) { }
		Size2F(float x, float y) : GenericSize(x, y) { }
	};

	using Vec2 = GenericVec<int32_t>;

	using Size2 = GenericSize<int32_t>;

	using Rect = GenericRect<int32_t>;
	using RectF = GenericRect<float>;
}

#endif