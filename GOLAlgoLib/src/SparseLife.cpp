#include <algorithm>

#include "Graphics2D.h"
#include "LifeAlgorithm.h"
#include "LifeHashSet.h"

gol::LifeHashSet gol::SparseLife(std::span<const Vec2> data, const Rect& bounds)
{
	constexpr static std::array dx = { -1,-1,-1,0,0,1,1,1 };
	constexpr static std::array dy = { -1,0,1,-1,1,-1,0,1 };

	ankerl::unordered_dense::map<Vec2, uint8_t> neighborCount;
	neighborCount.reserve(data.size() * 8);
	for (auto&& pos : data)
	{
		for (auto i = 0; i < 8; ++i)
		{
			const auto x = pos.X + dx[i];
			const auto y = pos.Y + dy[i];

			if (bounds.Width > 0 && bounds.Height > 0 && !bounds.InBounds(x, y))
				continue;

			++neighborCount[{x, y}];
		}
	}

	LifeHashSet newSet{};
	newSet.reserve(neighborCount.size());
	const LifeHashSet current{ data.begin(), data.end() };
	for (auto&& [pos, neighbors] : neighborCount)
	{
		if (neighbors == 3 || (neighbors == 2 && current.contains(pos)))
			newSet.insert(pos);
	}

	return newSet;
}