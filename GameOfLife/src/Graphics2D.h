#ifndef __Graphics2D_h__
#define __Graphics2D_h__

#include <cstdint>
#include <concepts>
#include <glm/glm.hpp>

#include "imgui.h"

namespace gol
{
	template <std::totally_ordered T>
	struct GenericVec
	{
		T X;
		T Y;

		GenericVec() : X(0), Y(0) { }
		GenericVec(T x, T y) : X(x), Y(y) { }

		auto operator<=>(const GenericVec<T>&) const = default;
	};

	template <std::totally_ordered T>
	struct GenericSize
	{
		T Width;
		T Height;


		GenericSize() : Width(0), Height(0) { }
		GenericSize(T width, T height) : Width(width), Height(height) { }
	};

	template <std::totally_ordered T>
	struct GenericRect
	{
		T X;
		T Y;
		T Width;
		T Height;

		inline GenericVec<T> Pos() const { return { X, Y }; }
		inline GenericSize<T> Size() const { return { Width, Height }; }

		inline GenericVec<T> UpperLeft() const { return { X, Y }; }
		inline GenericVec<T> UpperRight() const { return { X + Width, Y }; }
		inline GenericVec<T> LowerLeft() const { return { X, Y + Height }; }
		inline GenericVec<T> LowerRight() const { return { X + Width, Y + Height }; }

		GenericRect() : GenericRect(0, 0, 0, 0) { }
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
		Vec2F(glm::vec2 vec) : GenericVec(vec.x, vec.y) { }
		Vec2F(float x, float y) : GenericVec(x, y) { }

		operator ImVec2() const { return { X, Y }; }
		operator glm::vec2() const { return { X, Y }; }
	};

	struct Size2F : public GenericSize<float>
	{
		Size2F() : GenericSize() { }
		Size2F(ImVec2 vec) : GenericSize(vec.x, vec.y) { }
		Size2F(float x, float y) : GenericSize(x, y) { }

		operator ImVec2() const { return { Width, Height }; }
	};

	struct RectF : public GenericRect<float>
	{
		RectF() : GenericRect() { }
		RectF(const GenericRect<int>& rect) : RectF(
			static_cast<float>(rect.X), 
			static_cast<float>(rect.Y), 
			static_cast<float>(rect.Width), 
			static_cast<float>(rect.Height))
		{}
		RectF(float x, float y, float width, float height) : GenericRect(x, y, width, height) { }
		RectF(GenericVec<float> pos, GenericSize<float> size) : GenericRect(pos, size) {}

		operator GenericRect<int32_t>() const
		{
			return {
				static_cast<int32_t>(X),
				static_cast<int32_t>(Y),
				static_cast<int32_t>(Width),
				static_cast<int32_t>(Height),
			};
		}
	};

	using Vec2 = GenericVec<int32_t>;

	using Size2 = GenericSize<int32_t>;

	using Rect = GenericRect<int32_t>;
}

#endif