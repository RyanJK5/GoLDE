#ifndef __HashQuadtree_h__
#define __HashQuadtree_h__

#include <bit>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <iterator>
#include <limits>
#include <print>
#include <ranges>
#include <stack>
#include <stop_token>
#include <unordered_dense.h>
#include <utility>

#include "Graphics2D.h"
#include "LifeHashSet.h"

namespace gol {
template <int32_t Size> constexpr static int32_t Index2D(int32_t x, int32_t y) {
  return y * Size + x;
}

struct LifeNode {
  const LifeNode *NorthWest;
  const LifeNode *NorthEast;
  const LifeNode *SouthWest;
  const LifeNode *SouthEast;

  uint64_t Hash = 0ULL;
  uint64_t Population = 0ULL;
  bool IsEmpty = false;

  constexpr LifeNode(const LifeNode *nw, const LifeNode *ne, const LifeNode *sw,
                     const LifeNode *se, bool isLeaf = false)
      : NorthWest(nw), NorthEast(ne), SouthWest(sw), SouthEast(se) {
    if (!isLeaf) {
      IsEmpty =
          CheckEmpty(nw) && CheckEmpty(ne) && CheckEmpty(sw) && CheckEmpty(se);
      Population = NodePopulation(nw) + NodePopulation(ne) +
                   NodePopulation(sw) + NodePopulation(se);
    } else
      Population = 1;

    if consteval {
    } else {
      Hash = HashCombine(Hash, std::bit_cast<uint64_t>(NorthWest));
      Hash = HashCombine(Hash, std::bit_cast<uint64_t>(NorthEast));
      Hash = HashCombine(Hash, std::bit_cast<uint64_t>(SouthWest));
      Hash = HashCombine(Hash, std::bit_cast<uint64_t>(SouthEast));
    }
  }

private:
  constexpr static bool CheckEmpty(const LifeNode *n) {
    return n == nullptr || n->IsEmpty;
  }

  constexpr static uint64_t NodePopulation(const LifeNode *n) {
    return n ? n->Population : 0ULL;
  }

  constexpr static uint64_t HashCombine(uint64_t seed, uint64_t v) {
    return seed ^ (v + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
  }
};

struct NodeUpdateInfo {
  const LifeNode *Node;
  int64_t Generations;
};

struct LifeNodeEqual {
  using is_transparent = void;
  bool operator()(const LifeNode *lhs, const LifeNode *rhs) const {
    if (lhs == rhs)
      return true;
    if (!lhs || !rhs)
      return false;
    return lhs->NorthWest == rhs->NorthWest &&
           lhs->NorthEast == rhs->NorthEast &&
           lhs->SouthWest == rhs->SouthWest && lhs->SouthEast == rhs->SouthEast;
  }
};

struct LifeNodeHash {
  using is_transparent = void;
  size_t operator()(const gol::LifeNode *node) const;
};

constexpr inline LifeNode StaticTrueNode{nullptr, nullptr, nullptr, nullptr,
                                         true};

constexpr const LifeNode *TrueNode = &StaticTrueNode;
constexpr const LifeNode *FalseNode = nullptr;
} // namespace gol

namespace gol {
struct SlowKey {
  const LifeNode *Node;
  int64_t MaxAdvance = 0;
  auto operator<=>(const SlowKey &) const = default;
};

struct SlowHash {
  size_t operator()(const SlowKey &key) const noexcept;
};

struct HashLifeCache {
  ankerl::unordered_dense::map<const LifeNode *, const LifeNode *, LifeNodeHash,
                               LifeNodeEqual>
      NodeMap{};
  ankerl::unordered_dense::map<SlowKey, const LifeNode *, SlowHash> SlowCache{};
  ankerl::unordered_dense::map<int64_t, const LifeNode *> EmptyNodeCache{};
  std::deque<LifeNode> NodeStorage{};
};

class HashQuadtree {
private:
  template <typename T> class IteratorImpl {
  private:
    struct LifeNodeData {
      const LifeNode *node;
      Vec2L position;
      int64_t size;
      uint8_t quadrant;
    };

    constexpr static Vec2 ConvertToVec2(Vec2L pos) {
      return Vec2{static_cast<int32_t>(pos.X), static_cast<int32_t>(pos.Y)};
    }

    bool IsWithinBounds(Vec2L pos) const {
      constexpr static auto maxInt32 =
          static_cast<int64_t>(std::numeric_limits<int32_t>::max());
      constexpr static auto minInt32 =
          static_cast<int64_t>(std::numeric_limits<int32_t>::min());
      if (pos.X < minInt32 || pos.X > maxInt32 || pos.Y < minInt32 ||
          pos.Y > maxInt32)
        return false;
      if (!m_UseBounds)
        return true;
      const auto left = static_cast<int64_t>(m_Bounds.X);
      const auto top = static_cast<int64_t>(m_Bounds.Y);
      const auto right = left + m_Bounds.Width;
      const auto bottom = top + m_Bounds.Height;
      return pos.X >= left && pos.X < right && pos.Y >= top && pos.Y < bottom;
    }

    bool IntersectsBounds(Vec2L pos, int64_t size) const {
      if (!m_UseBounds)
        return true;
      const auto left = static_cast<int64_t>(m_Bounds.X);
      const auto top = static_cast<int64_t>(m_Bounds.Y);
      const auto right = left + m_Bounds.Width;
      const auto bottom = top + m_Bounds.Height;
      const auto regionRight = pos.X + size;
      const auto regionBottom = pos.Y + size;
      return !(regionRight <= left || pos.X >= right || regionBottom <= top ||
               pos.Y >= bottom);
    }

  public:
    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = std::remove_const_t<T>;
    using pointer = const value_type *;
    using reference = const value_type &;

    friend HashQuadtree;

    IteratorImpl()
        : m_Current{}, m_IsEnd(true), m_UseBounds(false), m_Bounds{} {}

    IteratorImpl<T> &operator++();
    IteratorImpl<T> operator++(int);

    bool operator==(const IteratorImpl<T> &other) const;
    bool operator!=(const IteratorImpl<T> &other) const;

    reference operator*() const;
    pointer operator->() const;

  private:
    IteratorImpl(const LifeNode *root, Vec2L offset, int64_t size, bool isEnd,
                 const Rect *bounds = nullptr);
    void AdvanceToNext();

  private:
    std::stack<LifeNodeData> m_Stack;
    value_type m_Current;
    bool m_IsEnd;
    bool m_UseBounds;
    Rect m_Bounds;
  };

public:
  HashQuadtree() = default;
  HashQuadtree(const LifeHashSet &data, Vec2 offset = {});

  HashQuadtree(const HashQuadtree &other);
  HashQuadtree &operator=(const HashQuadtree &other);

  ~HashQuadtree() = default;

public:
  using Iterator = IteratorImpl<Vec2>;
  using ConstIterator = IteratorImpl<const Vec2>;

  bool empty() const;

  Iterator begin();
  Iterator end();

  ConstIterator begin() const;
  ConstIterator end() const;

  Iterator begin(const Rect &bounds);
  ConstIterator begin(const Rect &bounds) const;

  uint64_t Population() const;

  int64_t Advance(int64_t maxAdvance = 0, std::stop_token stopToken = {});

  int32_t CalculateDepth() const;
  int64_t CalculateTreeSize() const;

  void PrepareCopyBetweenThreads();

  bool operator==(const HashQuadtree &other) const;
  bool operator!=(const HashQuadtree &other) const;

private:
  void Copy(const HashQuadtree &other);

  const LifeNode *ExpandUniverse(const LifeNode *node, int32_t level) const;
  bool NeedsExpansion(const LifeNode *node, int32_t level) const;

  NodeUpdateInfo AdvanceNode(std::stop_token stopToken, const LifeNode *node,
                             int32_t level, int64_t maxAdvance) const;

  const LifeNode *FindOrCreate(const LifeNode *nw, const LifeNode *ne,
                               const LifeNode *sw, const LifeNode *se) const;

  const LifeNode *BuildTreeRegion(std::span<Vec2L> cells, Vec2L pos,
                                  int64_t size);

  const LifeNode *EmptyTree(int64_t size) const;

  const LifeNode *BuildTree(const LifeHashSet &data);

  const LifeNode *CenteredHorizontal(const LifeNode &west,
                                     const LifeNode &east) const;

  const LifeNode *CenteredVertical(const LifeNode &north,
                                   const LifeNode &south) const;

  const LifeNode *CenteredSubNode(const LifeNode &node) const;

  template <int32_t Size, typename T>
  const LifeNode *Combine2x2(const T &nodes, int32_t topLeftX,
                             int32_t topLeftY) const;

  const LifeNode *AdvanceBase(const LifeNode *node) const;

  NodeUpdateInfo AdvanceSlow(std::stop_token stopToken, const LifeNode *node,
                             int32_t level, int64_t maxAdvance) const;

  NodeUpdateInfo AdvanceFast(std::stop_token stopToken, const LifeNode *node,
                             int32_t level, int64_t maxAdvance) const;

private:
  static inline thread_local HashLifeCache s_Cache{};

  mutable std::unique_ptr<HashLifeCache> m_TransferCache{};

  const LifeNode *m_Root = FalseNode;
  Vec2L m_RootOffset;
};

extern template class HashQuadtree::IteratorImpl<Vec2>;
extern template class HashQuadtree::IteratorImpl<const Vec2>;

template <typename T>
HashQuadtree::IteratorImpl<T>::IteratorImpl(const LifeNode *root, Vec2L offset,
                                            int64_t size, bool isEnd,
                                            const Rect *bounds)
    : m_Current(), m_IsEnd(isEnd), m_UseBounds(bounds != nullptr),
      m_Bounds(bounds ? *bounds : Rect{}) {
  if (!isEnd && root != FalseNode) {
    if (!m_UseBounds || IntersectsBounds(offset, size)) {
      m_Stack.push({root, offset, size, 0});
      AdvanceToNext();
    } else {
      m_IsEnd = true;
    }
  }
}

template <typename T> void HashQuadtree::IteratorImpl<T>::AdvanceToNext() {
  while (!m_Stack.empty()) {
    auto &frame = m_Stack.top();

    // If we're at a leaf (size == 1)
    if (frame.size == 1) {
      if (frame.node == TrueNode) {
        if (IsWithinBounds(frame.position)) {
          m_Current = ConvertToVec2(frame.position);
          m_Stack.pop();
          return; // Found a live cell within bounds
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
    const LifeNode *child = nullptr;
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

    if (child != FalseNode && !child->IsEmpty &&
        IntersectsBounds(childPos, halfSize)) {
      m_Stack.push({child, childPos, halfSize, 0});
    }
  }

  m_IsEnd = true; // No more live cells
}

template <typename T>
HashQuadtree::IteratorImpl<T> &HashQuadtree::IteratorImpl<T>::operator++() {
  AdvanceToNext();
  return *this;
}

template <typename T>
HashQuadtree::IteratorImpl<T> HashQuadtree::IteratorImpl<T>::operator++(int) {
  auto copy = *this;
  AdvanceToNext();
  return copy;
}

template <typename T>
bool HashQuadtree::IteratorImpl<T>::operator==(
    const IteratorImpl<T> &other) const {
  return m_IsEnd == other.m_IsEnd && (m_IsEnd || m_Current == other.m_Current);
}

template <typename T>
bool HashQuadtree::IteratorImpl<T>::operator!=(
    const IteratorImpl<T> &other) const {
  return !(*this == other);
}

template <typename T>
typename HashQuadtree::IteratorImpl<T>::reference
HashQuadtree::IteratorImpl<T>::operator*() const {
  return m_Current;
}

template <typename T>
typename HashQuadtree::IteratorImpl<T>::pointer
HashQuadtree::IteratorImpl<T>::operator->() const {
  return &m_Current;
}

template <int32_t Size, typename T>
const LifeNode *HashQuadtree::Combine2x2(const T &nodes, int32_t topLeftX,
                                         int32_t topLeftY) const {

  if constexpr (std::is_same_v<T, std::bitset<Size * Size>>) {
    constexpr static auto toNode = [](bool live) {
      return live ? TrueNode : FalseNode;
    };
    return FindOrCreate(
        toNode(nodes[Index2D<Size>(topLeftX, topLeftY)]),
        toNode(nodes[Index2D<Size>(topLeftX + 1, topLeftY)]),
        toNode(nodes[Index2D<Size>(topLeftX, topLeftY + 1)]),
        toNode(nodes[Index2D<Size>(topLeftX + 1, topLeftY + 1)]));
  } else {
    return FindOrCreate(nodes[Index2D<Size>(topLeftX, topLeftY)],
                        nodes[Index2D<Size>(topLeftX + 1, topLeftY)],
                        nodes[Index2D<Size>(topLeftX, topLeftY + 1)],
                        nodes[Index2D<Size>(topLeftX + 1, topLeftY + 1)]);
  }
}

} // namespace gol

#endif