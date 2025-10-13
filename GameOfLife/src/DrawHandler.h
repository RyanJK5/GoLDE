#ifndef __DrawManager_h__
#define __DrawManager_h__

#include <vector>

#include "Graphics2D.h"

namespace gol
{
	class DrawHandler
	{
	public:
		void DrawGrid(const std::vector<float>& positions) const;

		void DrawSelection(const RectF& bounds) const;

		void ClearBackground(const Rect& windowBounds, const Rect& viewportBounds) const;
	};
}

#endif