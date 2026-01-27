#ifndef __HashQuadtree_h__
#define __HashQuadtree_h__

#include <deque>
#include <iterator>
#include <ranges>
#include <cstddef>
#include <cstdint>
#include <stack>
#include <limits>
#include <unordered_dense.h>

#include "Graphics2D.h"
#include "LifeHashSet.h"

namespace gol 
{
	constexpr int64_t MaxAdvanceOf(int64_t stepSize);
	
	struct LifeNode 
    {
        const LifeNode* NorthWest;
        const LifeNode* NorthEast;
        const LifeNode* SouthWest;
        const LifeNode* SouthEast;
    
        uint64_t Hash;
        bool IsEmpty;
    
		constexpr LifeNode(
			const LifeNode* nw,
			const LifeNode* ne,
			const LifeNode* sw,
			const LifeNode* se,
			bool isLeaf = false
		)
		: NorthWest(nw), NorthEast(ne), SouthWest(sw), SouthEast(se), Hash(0ULL), IsEmpty(false)
		{
			if (!isLeaf)
			{
				auto check = [](const LifeNode* n) { return n == nullptr || n->IsEmpty; };
				IsEmpty = check(nw) && check(ne) && check(sw) && check(se);
			}

			if consteval {}
			else
			{
				Hash = HashCombine(Hash, reinterpret_cast<uint64_t>(NorthWest));
				Hash = HashCombine(Hash, reinterpret_cast<uint64_t>(NorthEast));
				Hash = HashCombine(Hash, reinterpret_cast<uint64_t>(SouthWest));
				Hash = HashCombine(Hash, reinterpret_cast<uint64_t>(SouthEast));
			}
		}
	private:
		static constexpr uint64_t HashCombine(uint64_t seed, uint64_t v)
		{
			return seed ^ (v + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
		}
    };

	struct NodeUpdateInfo
	{
		const LifeNode* Node;
		int64_t Generations;
	};

    struct LifeNodeEqual 
    {
        using is_transparent = void;
        bool operator()(const LifeNode* lhs, const LifeNode* rhs) const 
        {
            if (lhs == rhs) return true;
            if (!lhs || !rhs) return false;
            return lhs->NorthWest == rhs->NorthWest &&
                   lhs->NorthEast == rhs->NorthEast &&
                   lhs->SouthWest == rhs->SouthWest &&
                   lhs->SouthEast == rhs->SouthEast;
        }
    };

    struct LifeNodeHash 
    {
        using is_transparent = void;
    	size_t operator()(const gol::LifeNode* node) const;
    };

    constexpr inline LifeNode StaticTrueNode { nullptr, nullptr, nullptr, nullptr, true };

    constexpr const LifeNode* TrueNode = &StaticTrueNode;
    constexpr const LifeNode* FalseNode = nullptr;
}

namespace gol 
{
	struct HashLifeUpdateInfo;

    class HashQuadtree
    {
    private:
        template <typename T>    
        class IteratorImpl {    
        private:
            struct LifeNodeData {
                const LifeNode* node;
                Vec2L position;
                int64_t size;
                uint8_t quadrant;
            };

			constexpr static bool IsInBounds(Vec2L pos)
            {
                constexpr static auto maxInt32 = static_cast<int64_t>(std::numeric_limits<int32_t>::max());
                constexpr static auto minInt32 = static_cast<int64_t>(std::numeric_limits<int32_t>::min());
                return pos.X >= minInt32 && pos.X <= maxInt32 && pos.Y >= minInt32 && pos.Y <= maxInt32;
            }

            constexpr static Vec2 ConvertToVec2(Vec2L pos)
            {
                return Vec2{static_cast<int32_t>(pos.X), static_cast<int32_t>(pos.Y)};
            }

        public:
            using iterator_category = std::input_iterator_tag;
            using difference_type   = std::ptrdiff_t;
            using value_type        = std::remove_const_t<T>;
            using pointer           = const value_type*;
            using reference         = const value_type&;

            friend HashQuadtree;

            IteratorImpl() : m_Current{}, m_IsEnd(true) {}

            IteratorImpl<T>& operator++();
            IteratorImpl<T> operator++(int);

            bool operator==(const IteratorImpl<T>& other) const;
            bool operator!=(const IteratorImpl<T>& other) const;

            reference operator*() const;
            pointer operator->() const;
        private:
            IteratorImpl(const LifeNode* root, Vec2L offset, int64_t size, bool isEnd);
            void AdvanceToNext();
        private:
            std::stack<LifeNodeData> m_Stack;
            value_type m_Current;
            bool m_IsEnd;
        };
    public:
        HashQuadtree() = default;
		HashQuadtree(const LifeHashSet& data, Vec2 offset = {});
    public:
        using Iterator = IteratorImpl<Vec2>;
        using ConstIterator = IteratorImpl<const Vec2>;

        bool empty() const;

		Iterator begin();
		Iterator end();

		ConstIterator begin() const;
		ConstIterator end() const;

        HashLifeUpdateInfo NextGeneration(const Rect& bounds = {}, int64_t maxAdvance = 0) const;

		int32_t CalculateDepth() const;
		int64_t CalculateTreeSize() const;

		bool operator==(const HashQuadtree& other) const;
		bool operator!=(const HashQuadtree& other) const;
    private:
		HashQuadtree(const LifeNode* root, Vec2L offset);

		const LifeNode* ExpandUniverse(const LifeNode* node, int32_t level) const;
		bool NeedsExpansion(const LifeNode* node, int32_t level) const;

		NodeUpdateInfo AdvanceNode(const LifeNode* node, int32_t level, int64_t maxAdvance) const;

		const LifeNode* FindOrCreate(
            const LifeNode* nw, 
            const LifeNode* ne, 
            const LifeNode* sw,
            const LifeNode* se
        ) const;

		const LifeNode* BuildTreeRegion(
            std::span<Vec2L> cells, 
            Vec2L pos, int64_t size);

		const LifeNode* EmptyTree(int64_t size) const;

		const LifeNode* BuildTree(const LifeHashSet& data);

		const LifeNode* CenteredHorizontal(const LifeNode& west, const LifeNode& east) const;

		const LifeNode* CenteredVertical(const LifeNode& north, const LifeNode& south) const;

		const LifeNode* CenteredSubNode(const LifeNode& node) const;

		const LifeNode* AdvanceBase(const LifeNode* node) const;

		NodeUpdateInfo AdvanceSlow(const LifeNode* node, int32_t level, int64_t maxAdvance) const;

		NodeUpdateInfo AdvanceFast(const LifeNode* node, int32_t level, int64_t maxAdvance) const;
    private:
		struct SlowKey
		{
			const LifeNode* Node;
			int64_t MaxAdvance = 0;
			auto operator<=>(const SlowKey&) const = default;
		};

		struct SlowHash
		{
			size_t operator()(const SlowKey& key) const noexcept;
		};
	private:
		inline static ankerl::unordered_dense::map<
				const LifeNode*, const LifeNode*, LifeNodeHash, LifeNodeEqual> 
			s_NodeMap {};
		inline static ankerl::unordered_dense::map<
				SlowKey, const LifeNode*, SlowHash>
			s_SlowCache {};
		inline static ankerl::unordered_dense::map<int64_t, const LifeNode*> 
			s_EmptyNodeCache {};
		inline static std::deque<LifeNode>
			s_NodeStorage {};
	private:
		const LifeNode* m_Root = FalseNode;        
        Vec2L m_RootOffset;    
    };

	struct HashLifeUpdateInfo
	{
		HashQuadtree Data;
		int64_t Generations = 1;
	};

    extern template class HashQuadtree::IteratorImpl<Vec2>;
    extern template class HashQuadtree::IteratorImpl<const Vec2>;

	template <typename T>
	HashQuadtree::IteratorImpl<T>::IteratorImpl(
		const LifeNode* root, Vec2L offset, int64_t size, bool isEnd)
		: m_Current{}, m_IsEnd(isEnd)
	{
		if (!isEnd && root != FalseNode) {
			m_Stack.push({root, offset, size, 0});
			AdvanceToNext();
		}
	}

	template <typename T>
	void HashQuadtree::IteratorImpl<T>::AdvanceToNext()
	{
		while (!m_Stack.empty()) {
			auto& frame = m_Stack.top();
			
			// If we're at a leaf (size == 1)
			if (frame.size == 1) {
				if (frame.node == TrueNode) {
					if (IsInBounds(frame.position)) {
						m_Current = ConvertToVec2(frame.position);
						m_Stack.pop();
						return;  // Found a live cell within bounds
					}
					m_Stack.pop();
					continue;
				}
				m_Stack.pop();
				continue;
			}
			
			// Process next quadrant
			if (frame.quadrant >= 4) {
				m_Stack.pop();
				continue;
			}
			
			const auto halfSize = frame.size / 2;
			const LifeNode* child = nullptr;
			auto childPos = frame.position;
			
			switch (frame.quadrant++) {
				case 0: 
					child = frame.node->NorthWest; 
					break;
				case 1: 
					child = frame.node->NorthEast;
					childPos.X += halfSize;
					break;
				case 2: 
					child = frame.node->SouthWest;
					childPos.Y += halfSize;
					break;
				case 3: 
					child = frame.node->SouthEast;
					childPos.X += halfSize;
					childPos.Y += halfSize;
					break;
			}
            m_Stack.top().quadrant = frame.quadrant;
			
			if (child != FalseNode && !child->IsEmpty) {
				m_Stack.push({child, childPos, halfSize, 0});
			}
		}
		
		m_IsEnd = true;  // No more live cells
	}

	template <typename T>
	HashQuadtree::IteratorImpl<T>& HashQuadtree::IteratorImpl<T>::operator++()
	{
		AdvanceToNext();
		return *this;
	}

	template <typename T>
	HashQuadtree::IteratorImpl<T> HashQuadtree::IteratorImpl<T>::operator++(int)
	{
		auto copy = *this;
		AdvanceToNext();
		return copy;
	}

	template <typename T>
	bool HashQuadtree::IteratorImpl<T>::operator==(const IteratorImpl<T>& other) const
	{
		return m_IsEnd == other.m_IsEnd && 
			   (m_IsEnd || m_Current == other.m_Current);
	}

	template <typename T>
	bool HashQuadtree::IteratorImpl<T>::operator!=(const IteratorImpl<T>& other) const
	{
		return !(*this == other);
	}

	template <typename T>
	typename HashQuadtree::IteratorImpl<T>::reference HashQuadtree::IteratorImpl<T>::operator*() const
	{
		return m_Current;
	}

	template <typename T>
	typename HashQuadtree::IteratorImpl<T>::pointer HashQuadtree::IteratorImpl<T>::operator->() const
    {
		return &m_Current;
	}

	constexpr int64_t MaxAdvanceOf(int64_t stepSize)
	{
		if (stepSize == 0)
			return 0;

		auto power = 0ULL;
		while ((stepSize % (1ULL << power)) == 0)
			power++;
		return 1ULL << (power - 1ULL);
	}
}

#endif