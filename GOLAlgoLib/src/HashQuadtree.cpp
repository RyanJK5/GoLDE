#include <algorithm>
#include <array>
#include <memory>
#include <print>
#include <unordered_dense.h>
#include <vector>
#include <span>

#include "LifeAlgorithm.h"

namespace gol 
{
	HashLifeUpdateInfo HashLife(const HashQuadtree& data, const Rect& bounds, int64_t numSteps)
	{
		if (numSteps == 0)
			return data.NextGeneration(bounds, 0);

		const auto maxAdvance = MaxAdvanceOf(numSteps);
		
		HashQuadtree ret { data };
		auto generation = 0LL;
		while (generation < numSteps)
		{
			auto updateInfo = ret.NextGeneration(bounds, maxAdvance);
			ret = std::move(updateInfo.Data);
			generation += updateInfo.Generations;
		}

		return { .Data = ret, .Generations = generation };
	}

	size_t LifeNodeHash::operator()(const gol::LifeNode* node) const
	{
		if (!node) 
			return std::hash<const void*>{}(nullptr);
		return node->Hash;
	}

	template class HashQuadtree::IteratorImpl<Vec2>;
	template class HashQuadtree::IteratorImpl<const Vec2>;

	HashQuadtree::HashQuadtree(const LifeHashSet& data, Vec2 offset)
	{
		if (data.empty())
			return;
		
		m_Root = BuildTree(data);
		m_RootOffset += offset;
	}

	HashQuadtree::HashQuadtree(const LifeNode* root, Vec2 offset)
		: m_Root(root)
		, m_RootOffset(offset)
	{ }

	int32_t HashQuadtree::CalculateDepth() const
	{
		if (m_Root == FalseNode || m_Root == TrueNode)
			return 0;

		auto depth = 0;
		const LifeNode* current = m_Root;
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

		auto hashSet1 = *this | std::ranges::to<LifeHashSet>();
		auto hashSet2 = other | std::ranges::to<LifeHashSet>();

		return hashSet1 == hashSet2;
	}

	bool HashQuadtree::operator!=(const HashQuadtree& other) const
	{
		return !(*this == other);
	}

	int32_t HashQuadtree::CalculateTreeSize() const
	{
		if (m_Root == FalseNode)
			return 0;
		return 1 << CalculateDepth();
	}

	bool HashQuadtree::empty() const
	{
          return m_Root == FalseNode || m_Root->IsEmpty;
	}

	HashQuadtree::Iterator HashQuadtree::begin()
	{
		if (m_Root == FalseNode)
			return end();
		
		int32_t size = CalculateTreeSize();
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
		
		int32_t size = CalculateTreeSize();
		return ConstIterator(m_Root, m_RootOffset, size, false);
	}

	HashQuadtree::ConstIterator HashQuadtree::end() const
	{
		return ConstIterator{};
	}

	const LifeNode* HashQuadtree::FindOrCreate(const LifeNode* nw, const LifeNode* ne, const LifeNode* sw, const LifeNode* se) const
	{
		LifeNode toFind { nw, ne, sw, se };
		if (auto itr = s_NodeMap.find(&toFind); itr != s_NodeMap.end()) 
			return itr->first;

		auto node = std::make_unique<LifeNode>(nw, ne, sw, se);
		s_NodeStorage.push_back(std::move(node));
		s_NodeMap[s_NodeStorage.back().get()] = nullptr;
		return s_NodeStorage.back().get();
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

	NodeUpdateInfo HashQuadtree::AdvanceNode(const LifeNode* node, int32_t level, int64_t maxAdvance) const
	{
		if (node == FalseNode || level < 2)
			return { node, 0 };

		if (maxAdvance > 0)
		{
			const auto span = 1LL << level;
			if ((span / 4) > maxAdvance)
				return AdvanceSlow(node, level, maxAdvance);
		}
		return AdvanceFast(node, level, maxAdvance);
	}

	NodeUpdateInfo HashQuadtree::AdvanceSlow(const LifeNode* node, int32_t level, int64_t maxAdvance) const
	{
		if (node == FalseNode)
			return { FalseNode, 0 };
		if (level <= 2)
			return AdvanceFast(node, level, maxAdvance);

		constexpr static int subdivisions = 8;
		constexpr static auto index = [](int x, int y)
		{
			return y * subdivisions + x;
		};

		std::array<const LifeNode*, subdivisions * subdivisions> segments{};
		const auto fetchSegment = [&](int x, int y) -> const LifeNode*
		{
			const LifeNode* current = node;
			for (int bit = 2; bit >= 0 && current != FalseNode; --bit)
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

		for (int y = 0; y < subdivisions; ++y)
		{
			for (int x = 0; x < subdivisions; ++x)
			{
				segments[index(x, y)] = fetchSegment(x, y);
			}
		}

		const auto combine2x2 = [&](int startX, int startY) -> const LifeNode*
		{
			return FindOrCreate(
				segments[index(startX, startY)],
				segments[index(startX + 1, startY)],
				segments[index(startX, startY + 1)],
				segments[index(startX + 1, startY + 1)]
			);
		};

		const auto buildWindow = [&](int startX, int startY) -> const LifeNode*
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

		const auto result00 = AdvanceNode(window00, level - 1, maxAdvance);
		const auto result01 = AdvanceNode(window01, level - 1, maxAdvance);
		const auto result10 = AdvanceNode(window10, level - 1, maxAdvance);
		const auto result11 = AdvanceNode(window11, level - 1, maxAdvance);

		const auto generations = result00.Generations;
		const auto* combined = FindOrCreate(result00.Node, result01.Node, result10.Node, result11.Node);
		return { combined, generations };
	}

	NodeUpdateInfo HashQuadtree::AdvanceFast(const LifeNode* node, int32_t level, int64_t maxAdvance) const
	{
		if (node == FalseNode)
			return { FalseNode, 0 };

		if (auto itr = s_NodeMap.find(node); itr != s_NodeMap.end() && itr->second != nullptr)
		{
			const auto generations = 1 << (level - 2);
			return { itr->second, generations };
		}

		if (level == 2) 
		{
			const auto* base = AdvanceBase(node);
			s_NodeMap[node] = base;
			return { base, 1 };
		}
		
		const auto n00 = AdvanceNode(node->NorthWest, level - 1, maxAdvance);
		const auto n01 = AdvanceNode(CenteredHorizontal(*node->NorthWest, *node->NorthEast), level - 1, maxAdvance);
		const auto n02 = AdvanceNode(node->NorthEast, level - 1, maxAdvance);
		const auto n10 = AdvanceNode(CenteredVertical(*node->NorthWest, *node->SouthWest), level - 1, maxAdvance);
		const auto n11 = AdvanceNode(CenteredSubNode(*node), level - 1, maxAdvance);
		const auto n12 = AdvanceNode(CenteredVertical(*node->NorthEast, *node->SouthEast), level - 1, maxAdvance);
		const auto n20 = AdvanceNode(node->SouthWest, level - 1, maxAdvance);
		const auto n21 = AdvanceNode(CenteredHorizontal(*node->SouthWest, *node->SouthEast), level - 1, maxAdvance);
		const auto n22 = AdvanceNode(node->SouthEast, level - 1, maxAdvance);

		const auto tl = AdvanceNode(FindOrCreate(n00.Node, n01.Node, n10.Node, n11.Node), level - 1, maxAdvance);
		const auto tr = AdvanceNode(FindOrCreate(n01.Node, n02.Node, n11.Node, n12.Node), level - 1, maxAdvance);
		const auto bl = AdvanceNode(FindOrCreate(n10.Node, n11.Node, n20.Node, n21.Node), level - 1, maxAdvance);
		const auto br = AdvanceNode(FindOrCreate(n11.Node, n12.Node, n21.Node, n22.Node), level - 1, maxAdvance);

		const auto* result = FindOrCreate(tl.Node, tr.Node, bl.Node, br.Node);
		s_NodeMap[node] = result;
		const auto generations = 1 << (level - 2);
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
			return EmptyTree(1 << (level + 1));

		if (node == TrueNode)
			return FindOrCreate(FalseNode, FalseNode, FalseNode, TrueNode);

		const auto childSize = level > 0 ? (1 << (level - 1)) : 1;
		const auto* empty = EmptyTree(childSize);
		const auto* expandedNW = FindOrCreate(empty, empty, empty, node->NorthWest);
		const auto* expandedNE = FindOrCreate(empty, empty, node->NorthEast, empty);
		const auto* expandedSW = FindOrCreate(empty, node->SouthWest, empty, empty);
		const auto* expandedSE = FindOrCreate(node->SouthEast, empty, empty, empty);

		return FindOrCreate(expandedNW, expandedNE, expandedSW, expandedSE);
	}

	HashLifeUpdateInfo HashQuadtree::NextGeneration(const Rect& bounds, int64_t maxAdvance) const
	{
		if (m_Root == FalseNode)
			return {};

		const LifeNode* root = m_Root;
		
		auto level = CalculateDepth();
		auto size = std::max(1, CalculateTreeSize());
		auto offset = m_RootOffset;

		while (NeedsExpansion(root, level)) 
		{
			root = ExpandUniverse(root, level);
			const auto delta = std::max(1, size / 2);
			offset.X -= delta;
			offset.Y -= delta;
			level++;
			size <<= 1;
		}

		const auto advanced = AdvanceNode(root, level, maxAdvance);
		const auto centerDelta = std::max(1, size / 4);
		offset.X += centerDelta;
		offset.Y += centerDelta;

		const auto quadtree = HashQuadtree{ advanced.Node, offset };
		return HashLifeUpdateInfo
		{ 
			.Data = quadtree, 
			.Generations = static_cast<int64_t>(std::max(0LL, advanced.Generations)) 
		};
	}

	size_t HashQuadtree::QuadHash::operator()(const QuadKey& key) const noexcept
	{
		return ankerl::unordered_dense::detail::wyhash::hash(&key, sizeof(key));
	}

	const LifeNode* HashQuadtree::EmptyTree(int32_t size) const
	{
		if (size <= 1)
			return FalseNode;

        if (auto it = m_EmptyNodeCache.find(size); it != m_EmptyNodeCache.end()) 
            return it->second;

		auto child = EmptyTree(size / 2);
		auto result = FindOrCreate(child, child, child, child);
        m_EmptyNodeCache[size] = result;
        return result;
	}

	const LifeNode* HashQuadtree::BuildTreeRegion(
		std::span<Vec2> cells,
		Vec2 pos, int32_t size)
	{
		if (cells.empty())
			return EmptyTree(size);

		if (size == 1)
			return TrueNode;
		
		const auto half = size / 2;
		const auto midY = pos.Y + half;
		const auto midX = pos.X + half;
		
		auto itY = std::partition(cells.begin(), cells.end(), [midY](const Vec2& v) { return v.Y < midY; });
		std::span<Vec2> north = {cells.begin(), itY};
		std::span<Vec2> south = {itY, cells.end()};
		
		auto itNorthX = std::partition(north.begin(), north.end(), [midX](const Vec2& v) { return v.X < midX; });
		auto itSouthX = std::partition(south.begin(), south.end(), [midX](const Vec2& v) { return v.X < midX; });

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
			m_RootOffset = {0, 0};
			return FalseNode;
		}

		const auto& [minX, maxX] = std::ranges::minmax_element(cells, 
			[](const Vec2& a, const Vec2& b) { return a.X < b.X; });
		const auto& [minY, maxY] = std::ranges::minmax_element(cells, 
			[](const Vec2& a, const Vec2& b) { return a.Y < b.Y; });
		
		const auto neighborhoodSize = std::max(maxX->X - minX->X, maxY->Y - minY->Y) + 1;
		const auto gridExponent = static_cast<int32_t>(std::ceil(std::log2(neighborhoodSize)));
		const auto gridSize = 1 << gridExponent;
		
        m_RootOffset = {minX->X, minY->Y};
		
		std::vector<Vec2> cellVec{ cells.begin(), cells.end() };
		return BuildTreeRegion(cellVec, m_RootOffset, gridSize);
	}
}