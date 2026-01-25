#ifndef __HashQuadtree_h__
#define __HashQuadtree_h__

#include <iterator>
#include <ranges>
#include <cstddef>
#include <cstdint>
#include <stack>
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
                Vec2 position;
                int32_t size;
                uint8_t quadrant;
            };
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
            IteratorImpl(const LifeNode* root, Vec2 offset, int32_t size, bool isEnd);
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
		int32_t CalculateTreeSize() const;

		bool operator==(const HashQuadtree& other) const;
		bool operator!=(const HashQuadtree& other) const;
    private:
		HashQuadtree(const LifeNode* root, Vec2 offset);

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
            std::span<Vec2> cells, 
            Vec2 pos, int32_t size);

		const LifeNode* EmptyTree(int32_t size) const;

		const LifeNode* BuildTree(const LifeHashSet& data);

		const LifeNode* CenteredHorizontal(const LifeNode& west, const LifeNode& east) const;

		const LifeNode* CenteredVertical(const LifeNode& north, const LifeNode& south) const;

		const LifeNode* CenteredSubNode(const LifeNode& node) const;

		const LifeNode* AdvanceBase(const LifeNode* node) const;

		NodeUpdateInfo AdvanceSlow(const LifeNode* node, int32_t level, int64_t maxAdvance) const;

		NodeUpdateInfo AdvanceFast(const LifeNode* node, int32_t level, int64_t maxAdvance) const;
    private:
		struct QuadKey
		{
			Vec2 Offset;
			Vec2 Pos;
			int32_t Size = 0;
			auto operator<=>(const QuadKey&) const = default;
		};

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

		struct QuadHash
		{
			size_t operator()(const QuadKey& key) const noexcept;
		};
	private:
		inline static ankerl::unordered_dense::map<
			const LifeNode*, const LifeNode*, LifeNodeHash, LifeNodeEqual> 
		s_NodeMap {};
		inline static ankerl::unordered_dense::map<
			SlowKey, const LifeNode*, SlowHash>
			s_SlowCache {};

		inline static std::vector<std::unique_ptr<LifeNode>> s_NodeStorage {};
	private:
		ankerl::unordered_dense::map<QuadKey, const LifeNode*, QuadHash> m_TreeBuilderCache {};
        mutable ankerl::unordered_dense::map<int32_t, const LifeNode*> m_EmptyNodeCache {};
        
		const LifeNode* m_Root = FalseNode;        
        Vec2 m_RootOffset;    
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
		const LifeNode* root, Vec2 offset, int32_t size, bool isEnd)
		: m_Current(offset), m_IsEnd(isEnd)
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
					m_Current = frame.position;
					m_Stack.pop();
					return;  // Found a live cell
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