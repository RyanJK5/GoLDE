#ifndef UnboundedTopology_hpp_
#define UnboundedTopology_hpp_

#include "Topology.hpp"

namespace gol {
class UnboundedTopology : public Topology {
    std::string_view GetIdentifier() const override;

    bool CompatibleWith(LifeDataStructure& data) const override;

    int32_t Log2MaxIncrement(const BigInt& requestedStep) const override;

    void PrepareBorderCells(LifeDataStructure& data) override;

    void CleanupBorderCells(LifeDataStructure& data) override;
};
} // namespace gol

#endif