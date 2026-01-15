#include <algorithm>
#include <memory>
#include <print>
#include <unordered_dense.h>
#include <vector>
#include <span>

#include "LifeAlgorithm.h"

namespace gol 
{
	size_t LifeNodeHash::operator()(const gol::LifeNode* node) const
	{
		if (!node) 
			return std::hash<const void*>{}(nullptr);
		return node->Hash;
	}

	template class HashQuadtree::IteratorImpl<Vec2>;
	template class HashQuadtree::IteratorImpl<const Vec2>;

	HashQuadtree::HashQuadtree(const LifeHashSet& data)
	{
		if (!data.empty())
			m_Root = BuildTree(data);
	}

	int32_t HashQuadtree::CalculateTreeSize() const
	{
		if (m_Root == FalseNode || m_Root == TrueNode)
			return 1;
		
		int32_t depth = 0;
		const LifeNode* current = m_Root;
		while (current != TrueNode && current != FalseNode) {
			current = current->NorthWest;
			depth++;
		}
		return 1 << depth;
	}

	bool HashQuadtree::empty() const
	{
		return m_Root == FalseNode;
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
		return Iterator();
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
		return ConstIterator();
	}

	const LifeNode* HashQuadtree::FindOrCreate(const LifeNode* nw, const LifeNode* ne, const LifeNode* sw, const LifeNode* se) 
	{
		LifeNode toFind { nw, ne, sw, se };
		if (auto itr = m_NodeMap.find(&toFind); itr != m_NodeMap.end()) 
			return itr->first;

		auto node = std::make_unique<LifeNode>(nw, ne, sw, se);
		m_NodeStorage.push_back(std::move(node));
		m_NodeMap[m_NodeStorage.back().get()] = nullptr;
		return m_NodeStorage.back().get();
	}

	const LifeNode* HashQuadtree::CenteredSubnode(const LifeNode& node) 
	{
		return FindOrCreate(
			node.NorthWest->SouthEast,
			node.NorthEast->SouthWest,
			node.SouthWest->NorthEast,
			node.SouthEast->NorthWest
		);
	}

	const LifeNode* HashQuadtree::CenteredHorizontal(const LifeNode& west, const LifeNode& east) 
	{
		return FindOrCreate(
			west.NorthEast->SouthEast,
			east.NorthWest->SouthWest,
			west.SouthEast->NorthEast,
			east.SouthWest->NorthWest
		);
	}

	const LifeNode* HashQuadtree::CenteredVertical(const LifeNode& north, const LifeNode& south) 
	{
		return FindOrCreate(
			north.SouthWest->SouthEast,
			north.SouthEast->SouthWest,
			south.NorthWest->NorthEast,
			south.NorthEast->NorthWest
		);
	}

	const LifeNode* HashQuadtree::CenteredSubSubNode(const LifeNode& node) 
	{
		return FindOrCreate(
			node.NorthWest->SouthEast->SouthEast,
			node.NorthEast->SouthWest->SouthWest,
			node.SouthWest->NorthEast->NorthEast,
			node.SouthEast->NorthWest->NorthWest
		);
	}

	// TODO: Use more sophisticated algorithm for base case
	const LifeNode* HashQuadtree::AdvanceBase(const LifeNode* node) 
	{
		constexpr static auto gridSize = 4;
		const std::array cells = 
		{ 
			node->NorthWest->NorthWest, node->NorthWest->NorthEast, node->NorthEast->NorthWest, node->NorthEast->NorthEast,
			node->NorthWest->SouthWest, node->NorthWest->SouthEast, node->NorthEast->SouthWest, node->NorthEast->SouthWest,
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

		const auto* quadtree = BuildTree(result);
		return quadtree;
	}

	const LifeNode* HashQuadtree::AdvanceNode(const LifeNode* node, int32_t level) {
		return node;
	}

	const LifeNode* HashQuadtree::AdvanceFast(const LifeNode* node, int32_t level) 
	{
		if (auto itr = m_NodeMap.find(node); itr != m_NodeMap.end()) 
			return itr->second;

		if (level == 2) 
			return AdvanceBase(node);
		
		const auto n00 = CenteredSubnode(*node->NorthWest);
		const auto n01 = CenteredHorizontal(*node->NorthWest, *node->NorthEast);
		const auto n02 = CenteredSubnode(*node->NorthEast);
		const auto n10 = CenteredVertical(*node->NorthWest, *node->SouthWest);
		const auto n11 = CenteredSubSubNode(*node);
		const auto n12 = CenteredVertical(*node->NorthEast, *node->SouthEast);
		const auto n20 = CenteredSubnode(*node->SouthWest);
		const auto n21 = CenteredHorizontal(*node->SouthWest, *node->SouthEast);
		const auto n22 = CenteredSubnode(*node->SouthEast);

		return FindOrCreate(
			AdvanceNode(FindOrCreate(n00, n01, n10, n11), level - 1),
			AdvanceNode(FindOrCreate(n01, n02, n11, n12), level - 1),
			AdvanceNode(FindOrCreate(n10, n11, n20, n21), level - 1),
			AdvanceNode(FindOrCreate(n11, n12, n21, n22), level - 1)
		);
	}

	size_t HashQuadtree::QuadHash::operator()(const QuadKey& key) const noexcept
	{
		return ankerl::unordered_dense::detail::wyhash::hash(&key, sizeof(key));
	}

	const LifeNode* HashQuadtree::EmptyTree(int32_t size)
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
		const auto gridExponent = std::ceil(std::log2(neighborhoodSize));
		const auto gridSize = static_cast<int32_t>(std::pow(2, gridExponent));
		
        m_RootOffset = {minX->X, minY->Y};
		
		std::vector<Vec2> cellVec(cells.begin(), cells.end());
		return BuildTreeRegion(cellVec, m_RootOffset, gridSize);
	}
}