#include <algorithm>
#include <array>
#include <print>
#include <unordered_dense.h>
#include <cstdint>
#include <vector>
#include <span>

#include "LifeAlgorithm.h"

namespace gol 
{
	template <std::integral T>
	constexpr static int64_t Pow2(T exponent)
	{
		return 1LL << exponent;
	}

	constexpr static int64_t MaxAdvanceOf(int64_t stepSize)
	{
		if (stepSize == 0)
			return 0;

		auto power = 0LL;
		while (Pow2(power) <= stepSize)
			power++;
		return Pow2(power - 1LL);
	}

	int64_t HashLife(HashQuadtree& data, const Rect& bounds, int64_t numSteps, std::stop_token stopToken)
	{
		if (numSteps == 0)
			return data.Advance(bounds, 0, stopToken);

		auto generation = 0LL;
		while (generation < numSteps)
		{
			const auto maxAdvance = MaxAdvanceOf(numSteps - generation);
			
			const auto gens = data.Advance(bounds, maxAdvance, stopToken);
			if (gens == 0)
				return generation;
			generation += gens;
		}

		return generation;
	}

	size_t LifeNodeHash::operator()(const gol::LifeNode* node) const
	{
		if (!node) 
			return std::hash<const void*>{}(nullptr);
		return node->Hash;
	}

	size_t SlowHash::operator()(const SlowKey& key) const noexcept
	{
		uint64_t h1 = static_cast<uint64_t>(LifeNodeHash{}(key.Node));
		uint64_t h2 = static_cast<uint64_t>(static_cast<uint64_t>(key.MaxAdvance) + 0x9e3779b97f4a7c15ULL);

		uint64_t combined = h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));

		combined = (combined ^ (combined >> 30)) * 0xBF58476D1CE4E5B9ULL;
		combined = (combined ^ (combined >> 27)) * 0x94D049BB133111EBULL;
		combined = combined ^ (combined >> 31);

		return static_cast<size_t>(combined);
	}

	template class HashQuadtree::IteratorImpl<Vec2>;
	template class HashQuadtree::IteratorImpl<const Vec2>;

	HashQuadtree::HashQuadtree(const LifeHashSet& data, Vec2 offset)
	{
		if (data.empty())
			return;
		
		m_Root = BuildTree(data);
		m_RootOffset += {static_cast<int64_t>(offset.X), static_cast<int64_t>(offset.Y)};
	}

	HashQuadtree::HashQuadtree(const HashQuadtree& other)
	{
		Copy(other);
	}

	HashQuadtree& HashQuadtree::operator=(const HashQuadtree& other)
	{
		if (this != &other)
			Copy(other);
		return *this;
	}

	static const LifeNode* FindOrCreateFromCache(HashLifeCache& cache,
		const LifeNode* nw, const LifeNode* ne, const LifeNode* sw, const LifeNode* se)
	{
		LifeNode toFind{ nw, ne, sw, se };
		if (const auto itr = cache.NodeMap.find(&toFind); itr != cache.NodeMap.end())
			return itr->first;

		cache.NodeStorage.emplace_back(nw, ne, sw, se);
		cache.NodeMap[&cache.NodeStorage.back()] = nullptr;
		return &cache.NodeStorage.back();
	}

	using TransferMap = ankerl::unordered_dense::map<const LifeNode*, const LifeNode*>;
	static const LifeNode* BuildCache(
		TransferMap& transferMap,
		HashLifeCache& cache,
		const LifeNode* original)
	{
		if (original == FalseNode)
			return FalseNode;
		if (original == TrueNode)
			return TrueNode;

		if (const auto it = transferMap.find(original); it != transferMap.end())
			return it->second;

		const auto* nw = BuildCache(transferMap, cache, original->NorthWest);
		const auto* ne = BuildCache(transferMap, cache, original->NorthEast);
		const auto* sw = BuildCache(transferMap, cache, original->SouthWest);
		const auto* se = BuildCache(transferMap, cache, original->SouthEast);

		const auto* ret = FindOrCreateFromCache(cache, nw, ne, sw, se);
		transferMap[original] = ret;
		return ret;
	}

	void HashQuadtree::Copy(const HashQuadtree& other)
	{
		m_Root = FalseNode;
		m_RootOffset = other.m_RootOffset;

		if (other.m_Root == FalseNode)
			return;
		
		if (other.m_TransferCache)
		{
			TransferMap transferMap{};
			BuildCache(transferMap, s_Cache, &other.m_TransferCache->NodeStorage.back());
			m_Root = &s_Cache.NodeStorage.back();
		}
		else
			m_Root = other.m_Root;
	}

	void HashQuadtree::PrepareCopy()
	{
		TransferMap transferMap{};
		m_TransferCache = std::make_unique<HashLifeCache>();
		BuildCache(transferMap, *m_TransferCache, m_Root);
	}

	int32_t HashQuadtree::CalculateDepth() const
	{
		if (m_Root == FalseNode || m_Root == TrueNode)
			return 0;

		auto depth = 0;
		const auto* current = m_Root;
		while (current != TrueNode && current != FalseNode)
		{
			current = current->NorthWest;
			depth++;
		}
		return depth;
	}

	bool HashQuadtree::operator==(const HashQuadtree& other) const
	{
		if (m_Root == other.m_Root && m_RootOffset == other.m_RootOffset)
			return true;

		const auto hashSet1 = *this | std::ranges::to<LifeHashSet>();
		const auto hashSet2 = other | std::ranges::to<LifeHashSet>();

		return hashSet1 == hashSet2;
	}

	bool HashQuadtree::operator!=(const HashQuadtree& other) const
	{
		return !(*this == other);
	}

	int64_t HashQuadtree::CalculateTreeSize() const
	{
		if (m_Root == FalseNode)
			return 0;
		return Pow2(CalculateDepth());
	}

	bool HashQuadtree::empty() const
	{
          return m_Root == FalseNode || m_Root->IsEmpty;
	}

	uint64_t HashQuadtree::Population() const
	{
		return m_Root ? m_Root->Population : 0ULL;
	}

	HashQuadtree::Iterator HashQuadtree::begin()
	{
		if (m_Root == FalseNode)
			return end();
		
		const auto size = CalculateTreeSize();
		return Iterator { m_Root, m_RootOffset, size, false };
	}

	HashQuadtree::Iterator HashQuadtree::end()
	{
		return Iterator{};
	}

	HashQuadtree::ConstIterator HashQuadtree::begin() const 
	{
		if (m_Root == FalseNode)
			return end();
		
		const auto size = CalculateTreeSize();
		return ConstIterator{ m_Root, m_RootOffset, size, false };
	}

	HashQuadtree::Iterator HashQuadtree::begin(const Rect& bounds)
	{
		if (m_Root == FalseNode)
			return end();

		const auto size = CalculateTreeSize();
		return Iterator { m_Root, m_RootOffset, size, false, &bounds };
	}

	HashQuadtree::ConstIterator HashQuadtree::begin(const Rect& bounds) const
	{
		if (m_Root == FalseNode)
			return end();

		const auto size = CalculateTreeSize();
		return ConstIterator{ m_Root, m_RootOffset, size, false, &bounds };
	}

	HashQuadtree::ConstIterator HashQuadtree::end() const
	{
		return ConstIterator{};
	}

	const LifeNode* HashQuadtree::FindOrCreate( 
		const LifeNode* nw, const LifeNode* ne, const LifeNode* sw, const LifeNode* se) const
	{
		return FindOrCreateFromCache(s_Cache, nw, ne, sw, se);
	}

	const LifeNode* HashQuadtree::CenteredHorizontal(const LifeNode& west, const LifeNode& east) const
	{
		return FindOrCreate(
			west.NorthEast,
			east.NorthWest,
			west.SouthEast,
			east.SouthWest
		);
	}

	const LifeNode* HashQuadtree::CenteredVertical(const LifeNode& north, const LifeNode& south) const
	{
		return FindOrCreate(
			north.SouthWest,
			north.SouthEast,
			south.NorthWest,
			south.NorthEast
		);
	}

	const LifeNode* HashQuadtree::CenteredSubNode(const LifeNode& node) const
	{
		return FindOrCreate(
			node.NorthWest->SouthEast,
			node.NorthEast->SouthWest,
			node.SouthWest->NorthEast,
			node.SouthEast->NorthWest
		);
	}

	// TODO: Use more sophisticated algorithm for base case
	const LifeNode* HashQuadtree::AdvanceBase(const LifeNode* node) const
	{
		constexpr static auto gridSize = 4;
		const std::array cells = 
		{
			node->NorthWest->NorthWest, node->NorthWest->NorthEast, node->NorthEast->NorthWest, node->NorthEast->NorthEast,
			node->NorthWest->SouthWest, node->NorthWest->SouthEast, node->NorthEast->SouthWest, node->NorthEast->SouthEast,
			node->SouthWest->NorthWest, node->SouthWest->NorthEast, node->SouthEast->NorthWest, node->SouthEast->NorthEast,
			node->SouthWest->SouthWest, node->SouthWest->SouthEast, node->SouthEast->SouthWest, node->SouthEast->SouthEast
		};

		LifeHashSet result {};
		for (auto x = 0; x < gridSize; x++) 
		{
			for (auto y = 0; y < gridSize; y++)
			{
				auto index = y * gridSize + x;
				if (cells[index] == TrueNode)
					result.insert({x, y});
			}
		}
		result = SparseLife(result, {0, 0, gridSize, gridSize});

		return FindOrCreate(
			result.contains({ 1, 1 }) ? TrueNode : FalseNode,
			result.contains({ 2, 1 }) ? TrueNode : FalseNode,
			result.contains({ 1, 2 }) ? TrueNode : FalseNode,
			result.contains({ 2, 2 }) ? TrueNode : FalseNode
		);
	}

	NodeUpdateInfo HashQuadtree::AdvanceNode(std::stop_token stopToken, const LifeNode* node, int32_t level, int64_t maxAdvance) const
	{
		if (stopToken.stop_requested())
			return { node, 0 };
		if (node == FalseNode || level < 2)
			return { node, 0 };

		if (maxAdvance > 0)
		{
			const auto span = Pow2(level);
			if ((span / 4) > maxAdvance)
				return AdvanceSlow(stopToken, node, level, maxAdvance);
		}
		return AdvanceFast(stopToken, node, level, maxAdvance);
	}

	NodeUpdateInfo HashQuadtree::AdvanceSlow(std::stop_token stopToken, const LifeNode* node, int32_t level, int64_t maxAdvance) const
	{
		if (node == FalseNode)
			return { FalseNode, 0 };
		if (level <= 2)
			return AdvanceFast(stopToken, node, level, maxAdvance);
		if (const auto it = s_Cache.SlowCache.find({ node, maxAdvance }); it != s_Cache.SlowCache.end())
			return { it->second, maxAdvance };

		constexpr static auto subdivisions = 8;
		constexpr static auto index = [](int32_t x, int32_t y)
		{
			return y * subdivisions + x;
		};

		std::array<const LifeNode*, subdivisions * subdivisions> segments{};
		const auto fetchSegment = [&](int32_t x, int32_t y) -> const LifeNode*
		{
			const LifeNode* current = node;
			for (auto bit = 2; bit >= 0 && current != FalseNode; --bit)
			{
				const bool east = (x >> bit) & 1;
				const bool south = (y >> bit) & 1;
				if (south)
					current = east ? current->SouthEast : current->SouthWest;
				else
					current = east ? current->NorthEast : current->NorthWest;
				if (current == nullptr)
					break;
			}
			return current;
		};

		for (auto y = 0; y < subdivisions; ++y)
		{
			for (auto x = 0; x < subdivisions; ++x)
				segments[index(x, y)] = fetchSegment(x, y);
		}

		const auto combine2x2 = [&](int32_t startX, int32_t startY) -> const LifeNode*
		{
			return FindOrCreate(
				segments[index(startX, startY)],
				segments[index(startX + 1, startY)],
				segments[index(startX, startY + 1)],
				segments[index(startX + 1, startY + 1)]
			);
		};

		const auto buildWindow = [&](int32_t startX, int32_t startY) -> const LifeNode*
		{
			const auto* nw = combine2x2(startX, startY);
			const auto* ne = combine2x2(startX + 2, startY);
			const auto* sw = combine2x2(startX, startY + 2);
			const auto* se = combine2x2(startX + 2, startY + 2);
			return FindOrCreate(nw, ne, sw, se);
		};

		const auto* window00 = buildWindow(1, 1);
		const auto* window01 = buildWindow(3, 1);
		const auto* window10 = buildWindow(1, 3);
		const auto* window11 = buildWindow(3, 3);

		const auto result00 = AdvanceNode(stopToken, window00, level - 1, maxAdvance);
		const auto result01 = AdvanceNode(stopToken, window01, level - 1, maxAdvance);
		const auto result10 = AdvanceNode(stopToken, window10, level - 1, maxAdvance);
		const auto result11 = AdvanceNode(stopToken, window11, level - 1, maxAdvance);

		const auto generations = result00.Generations;
		const auto* combined = FindOrCreate(result00.Node, result01.Node, result10.Node, result11.Node);

		s_Cache.SlowCache[{ node, generations }] = combined;
		return { combined, generations };
	}

	NodeUpdateInfo HashQuadtree::AdvanceFast(std::stop_token stopToken, const LifeNode* node, int32_t level, int64_t maxAdvance) const
	{
		if (node == FalseNode)
			return { FalseNode, 0 };

		if (const auto itr = s_Cache.NodeMap.find(node); itr != s_Cache.NodeMap.end() && itr->second != nullptr)
		{
			const auto generations = Pow2(level - 2);
			return { itr->second, generations };
		}

		if (level == 2) 
		{
			const auto* base = AdvanceBase(node);
			s_Cache.NodeMap[node] = base;
			return { base, 1 };
		}
		
		const auto n00 = AdvanceNode(stopToken, node->NorthWest, level - 1, maxAdvance);
		const auto n01 = AdvanceNode(stopToken, CenteredHorizontal(*node->NorthWest, *node->NorthEast), level - 1, maxAdvance);
		const auto n02 = AdvanceNode(stopToken, node->NorthEast, level - 1, maxAdvance);
		const auto n10 = AdvanceNode(stopToken, CenteredVertical(*node->NorthWest, *node->SouthWest), level - 1, maxAdvance);
		const auto n11 = AdvanceNode(stopToken, CenteredSubNode(*node), level - 1, maxAdvance);
		const auto n12 = AdvanceNode(stopToken, CenteredVertical(*node->NorthEast, *node->SouthEast), level - 1, maxAdvance);
		const auto n20 = AdvanceNode(stopToken, node->SouthWest, level - 1, maxAdvance);
		const auto n21 = AdvanceNode(stopToken, CenteredHorizontal(*node->SouthWest, *node->SouthEast), level - 1, maxAdvance);
		const auto n22 = AdvanceNode(stopToken, node->SouthEast, level - 1, maxAdvance);

		const auto tl = AdvanceNode(stopToken, FindOrCreate(n00.Node, n01.Node, n10.Node, n11.Node), level - 1, maxAdvance);
		const auto tr = AdvanceNode(stopToken, FindOrCreate(n01.Node, n02.Node, n11.Node, n12.Node), level - 1, maxAdvance);
		const auto bl = AdvanceNode(stopToken, FindOrCreate(n10.Node, n11.Node, n20.Node, n21.Node), level - 1, maxAdvance);
		const auto br = AdvanceNode(stopToken, FindOrCreate(n11.Node, n12.Node, n21.Node, n22.Node), level - 1, maxAdvance);

		const auto* result = FindOrCreate(tl.Node, tr.Node, bl.Node, br.Node);
		s_Cache.NodeMap[node] = result;
		const auto generations = Pow2(level - 2);
		return { result, generations };
	}

	

	bool HashQuadtree::NeedsExpansion(const LifeNode* node, int32_t level) const
	{
		if (node == FalseNode)
			return false;
		if (level <= 2)
			return true;

		constexpr static auto notEmpty = [](const LifeNode* n)
		{
			return n != FalseNode && !n->IsEmpty;
		};

		const auto* nw = node->NorthWest;
		if (notEmpty(nw)) 
		{ 
			if (notEmpty(nw->NorthWest) || notEmpty(nw->NorthEast) || notEmpty(nw->SouthWest))
				return true;

			const auto* nw_se = nw->SouthEast;
			if (notEmpty(nw_se) && (notEmpty(nw_se->NorthWest) || notEmpty(nw_se->NorthEast) || notEmpty(nw_se->SouthWest)))
				return true;
		}

		const auto* ne = node->NorthEast;
		if (notEmpty(ne)) 
		{
			if (notEmpty(ne->NorthWest) || notEmpty(ne->NorthEast) || notEmpty(ne->SouthEast))
				return true;

			const auto* ne_sw = ne->SouthWest;
			if (notEmpty(ne_sw) && (notEmpty(ne_sw->NorthWest) || notEmpty(ne_sw->NorthEast) || notEmpty(ne_sw->SouthEast)))
				return true;
		}

		const auto* sw = node->SouthWest;
		if (notEmpty(sw)) 
		{
			if (notEmpty(sw->NorthWest) || notEmpty(sw->SouthWest) || notEmpty(sw->SouthEast))
				return true;

			const auto* sw_ne = sw->NorthEast;
			if (notEmpty(sw_ne) && (notEmpty(sw_ne->NorthWest) || notEmpty(sw_ne->SouthWest) || notEmpty(sw_ne->SouthEast)))
				return true;
		}

		const auto* se = node->SouthEast;
		if (notEmpty(se)) 
		{
			if (notEmpty(se->NorthEast) || notEmpty(se->SouthWest) || notEmpty(se->SouthEast))
				return true;

			const auto* se_nw = se->NorthWest;
			if (notEmpty(se_nw) && (notEmpty(se_nw->NorthEast) || notEmpty(se_nw->SouthWest) || notEmpty(se_nw->SouthEast)))
				return true;
		}

		return false;
	}

	const LifeNode* HashQuadtree::ExpandUniverse(const LifeNode* node, int32_t level) const
	{
		if (node == FalseNode)
			return EmptyTree(Pow2(level + 1));

		if (node == TrueNode)
			return FindOrCreate(FalseNode, FalseNode, FalseNode, TrueNode);

		const auto childSize = level > 0 ? Pow2(level - 1) : 1LL;
		const auto* empty = EmptyTree(childSize);
		const auto* expandedNW = FindOrCreate(empty, empty, empty, node->NorthWest);
		const auto* expandedNE = FindOrCreate(empty, empty, node->NorthEast, empty);
		const auto* expandedSW = FindOrCreate(empty, node->SouthWest, empty, empty);
		const auto* expandedSE = FindOrCreate(node->SouthEast, empty, empty, empty);

		return FindOrCreate(expandedNW, expandedNE, expandedSW, expandedSE);
	}

	int64_t HashQuadtree::Advance([[maybe_unused]] const Rect& bounds, int64_t maxAdvance, std::stop_token stopToken)
	{
		if (m_Root == FalseNode)
			return {};

		const auto* root = m_Root;
		
		auto depth = CalculateDepth();
		auto size = std::max(1LL, CalculateTreeSize());
		auto offset = m_RootOffset;

		while (NeedsExpansion(root, depth) || (size / 4 < maxAdvance)) 
		{
			root = ExpandUniverse(root, depth);
			const auto delta = std::max(1LL, size / 2);
			offset.X -= delta;
			offset.Y -= delta;
			depth++;
			size = Pow2(depth);
		}

		if (depth >= 63)
			return 0;
		const auto advanced = AdvanceNode(stopToken, root, depth, maxAdvance);
		const auto centerDelta = std::max(1LL, size / 4);
		offset.X += centerDelta;
		offset.Y += centerDelta;

		m_Root = advanced.Node;
		m_RootOffset = offset;
		return std::max(static_cast<int64_t>(0), advanced.Generations);
	}

	const LifeNode* HashQuadtree::EmptyTree(int64_t size) const
	{
		if (size <= 1)
			return FalseNode;

        if (const auto it = s_Cache.EmptyNodeCache.find(size); it != s_Cache.EmptyNodeCache.end()) 
            return it->second;

		const auto child = EmptyTree(size / 2);
		const auto result = FindOrCreate(child, child, child, child);
        s_Cache.EmptyNodeCache[size] = result;
        return result;
	}

	const LifeNode* HashQuadtree::BuildTreeRegion(
		std::span<Vec2L> cells,
		Vec2L pos, int64_t size)
	{
		if (cells.empty())
			return EmptyTree(size);

		if (size == 1)
			return TrueNode;
		
		const auto half = size / 2;
		const auto midY = pos.Y + half;
		const auto midX = pos.X + half;
		
		using std::ranges::partition;

		const auto itY = partition(cells, [midY](const Vec2L& v) { return v.Y < midY; }).begin();
		const std::span<Vec2L> north {cells.begin(), itY};
		const std::span<Vec2L> south {itY, cells.end()};
		
		const auto itNorthX = partition(north, [midX](const Vec2L& v) { return v.X < midX; }).begin();
		const auto itSouthX = partition(south, [midX](const Vec2L& v) { return v.X < midX; }).begin();

		return FindOrCreate(
			BuildTreeRegion({north.begin(), itNorthX}, {pos.X, pos.Y}, half),
			BuildTreeRegion({itNorthX, north.end()}, {pos.X + half, pos.Y}, half),
			BuildTreeRegion({south.begin(), itSouthX}, {pos.X, pos.Y + half}, half),
			BuildTreeRegion({itSouthX, south.end()}, {pos.X + half, pos.Y + half}, half)
		);
	}

	const LifeNode* HashQuadtree::BuildTree(const LifeHashSet& cells) 
	{
		if (cells.empty())
		{
			m_RootOffset = Vec2L{0, 0};
			return FalseNode;
		}

		const auto [minX, maxX] = std::ranges::minmax_element(cells, 
			[](Vec2 a, Vec2 b) { return a.X < b.X; });
		const auto [minY, maxY] = std::ranges::minmax_element(cells, 
			[](Vec2 a, Vec2 b) { return a.Y < b.Y; });
		
		const auto neighborhoodSize = std::max(
			static_cast<int64_t>(maxX->X) - minX->X, 
			static_cast<int64_t>(maxY->Y) - minY->Y) + 1;
		const auto gridExponent = static_cast<int64_t>(std::ceil(std::log2(neighborhoodSize)));
		const auto gridSize = Pow2(gridExponent);
		
        m_RootOffset = Vec2L{static_cast<int64_t>(minX->X), static_cast<int64_t>(minY->Y)};
		
		std::vector<Vec2L> cellVec;
		cellVec.reserve(cells.size());
		for (const auto cell : cells)
			cellVec.emplace_back(static_cast<int64_t>(cell.X), static_cast<int64_t>(cell.Y));

		return BuildTreeRegion(cellVec, m_RootOffset, gridSize);
	}
}