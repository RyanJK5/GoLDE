#ifndef __LifeAlgorithm_h__
#define __LifeAlgorithm_h__

#include <functional>
#include <span>
#include <variant>

#include "Graphics2D.h"
#include "HashQuadtree.h"
#include "LifeHashSet.h"

namespace gol
{
	LifeHashSet SparseLife(std::span<const Vec2> data, const Rect& bounds);

	HashLifeUpdateInfo HashLife(const HashQuadtree& data, const Rect& bounds, int64_t numSteps);

	enum class LifeAlgorithm {
		SparseLife,
		HashLife
	};
}

#endif