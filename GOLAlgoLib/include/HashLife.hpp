#ifndef HashLife_hpp_
#define HashLife_hpp_

#include <concepts>

#include "HashQuadtree.hpp"
#include "LifeAlgorithm.hpp"

namespace gol {

class HashLife : public LifeAlgorithm, AlgorithmRegistrator<HashLife> {
  public:
    static std::string_view Identifier;

    HashLife();

    void SetTopology(std::unique_ptr<Topology> topology) override;

    void SetRule(const LifeRule& rule) override;

    bool CompatibleWith(const LifeDataStructure& data) const override;

    BigInt Step(LifeDataStructure& data, const BigInt& numSteps,
                std::stop_token stopToken = {}) override;

  private:
    int32_t DoOneJump(HashQuadtree& data, int32_t advanceLevel,
                      std::stop_token stopToken);

    NodeUpdateInfo AdvanceNode(const HashQuadtree& data,
                               std::stop_token stopToken, const LifeNode* node,
                               int32_t level, int32_t advanceDepth) const;

    NodeUpdateInfo AdvanceSlow(const HashQuadtree& data,
                               std::stop_token stopToken, const LifeNode* node,
                               int32_t level, int32_t advanceLevel) const;

    NodeUpdateInfo AdvanceFast(const HashQuadtree& data,
                               std::stop_token stopToken, const LifeNode* node,
                               int32_t level, int32_t advanceLevel) const;

    const LifeNode* AdvanceBase(const HashQuadtree& data,
                                const LifeNode* node) const;

    const LifeNode* AdvanceBaseOneGen(const HashQuadtree& data,
                                      const LifeNode* node) const;

    struct FirstGenResults {
        uint16_t nw, n, ne, w, center, e, sw, s, se;
    };

    // Extracts the four 16-bit quadrant encodings from a level-3 node.
    struct LeafQuadrants {
        uint16_t nw, ne, sw, se;
    };

    FirstGenResults ComputeFirstGeneration(const LeafQuadrants& q) const;
    uint16_t AssembleCentered6x6(const FirstGenResults& gen1) const;

    LeafQuadrants EncodeLevel3(const LifeNode* node) const;

  private:
    std::unique_ptr<Topology> m_Topology;

    static thread_local LifeRule s_Rule;

    // The cache for the HashLife algorithm when the step size is bounded.
    static thread_local ankerl::unordered_dense::map<SlowKey, const LifeNode*,
                                                     SlowHash>
        s_SlowCache;
};
} // namespace gol

#endif