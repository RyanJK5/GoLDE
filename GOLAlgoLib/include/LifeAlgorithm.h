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
	LifeHashSet SparseLife(std::span<const Vec2> data, const Rect& bounds, std::stop_token = {});

	int64_t HashLife(HashQuadtree& data, const Rect& bounds, int64_t numSteps, std::stop_token stopToken = {});

	enum class LifeAlgorithm {
		SparseLife,
		HashLife
	};
}

#endif