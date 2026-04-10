#include "UnboundedTopology.hpp"
#include "HashQuadtree.hpp"
#include "LifeNode.hpp"

namespace gol {
std::string_view UnboundedTopology::GetIdentifier() const {
    return "Unbounded";
}

bool UnboundedTopology::CompatibleWith(LifeDataStructure& data) const {
    return typeid(HashQuadtree) == typeid(data);
}

int32_t UnboundedTopology::Log2MaxIncrement(const BigInt& requestedStep) const {
    if (requestedStep.is_zero())
        return -1;

    return static_cast<int32_t>(boost::multiprecision::msb(requestedStep));
}

static bool NeedsExpansion(const LifeNode* node, int32_t level) {
    if (node == FalseNode)
        return false;
    if (level <= 3)
        return true;

    const auto notEmpty = [&](const LifeNode* n) {
        return n != FalseNode && !n->IsEmpty;
    };

    // We simply want to consider if the outer rim of cells is completely empty,
    // and if the next level down is completely empty. This may sometimes cause
    // unnecessary expansion, but it is a fairly low-overhead procedure that may
    // actually result in better performance when hyper speed is enabled.

    const auto* nw = node->NorthWest;
    if (notEmpty(nw)) {
        if (notEmpty(nw->NorthWest) || notEmpty(nw->NorthEast) ||
            notEmpty(nw->SouthWest))
            return true;

        const auto* nwSe = nw->SouthEast;
        if (notEmpty(nwSe) &&
            (notEmpty(nwSe->NorthWest) || notEmpty(nwSe->NorthEast) ||
             notEmpty(nwSe->SouthWest)))
            return true;
    }

    const auto* ne = node->NorthEast;
    if (notEmpty(ne)) {
        if (notEmpty(ne->NorthWest) || notEmpty(ne->NorthEast) ||
            notEmpty(ne->SouthEast))
            return true;

        const auto* neSw = ne->SouthWest;
        if (notEmpty(neSw) &&
            (notEmpty(neSw->NorthWest) || notEmpty(neSw->NorthEast) ||
             notEmpty(neSw->SouthEast)))
            return true;
    }

    const auto* sw = node->SouthWest;
    if (notEmpty(sw)) {
        if (notEmpty(sw->NorthWest) || notEmpty(sw->SouthWest) ||
            notEmpty(sw->SouthEast))
            return true;

        const auto* swNe = sw->NorthEast;
        if (notEmpty(swNe) &&
            (notEmpty(swNe->NorthWest) || notEmpty(swNe->SouthWest) ||
             notEmpty(swNe->SouthEast)))
            return true;
    }

    const auto* se = node->SouthEast;
    if (notEmpty(se)) {
        if (notEmpty(se->NorthEast) || notEmpty(se->SouthWest) ||
            notEmpty(se->SouthEast))
            return true;

        const auto* seNw = se->NorthWest;
        if (notEmpty(seNw) &&
            (notEmpty(seNw->NorthEast) || notEmpty(seNw->SouthWest) ||
             notEmpty(seNw->SouthEast)))
            return true;
    }

    return false;
}

void UnboundedTopology::PrepareBorderCells(LifeDataStructure& data) {
    // Before we can actually advance, we must make sure the universe is
    // sufficiently large so that no data is lost after advancing (remember that
    // HashLife will cut off 75% of the universe's area)

    auto& hashQuadtree = dynamic_cast<HashQuadtree&>(data);

    const auto* root = hashQuadtree.Data();
    auto depth = hashQuadtree.CalculateDepth();

    while (NeedsExpansion(root, depth)) {
        root = hashQuadtree.ExpandUniverse(root, depth);
        depth++;
    }

    hashQuadtree.OverwriteData(root, depth);
}

void UnboundedTopology::CleanupBorderCells(LifeDataStructure& data) {
    // No need to do any extra work in an unbounded universe
}
} // namespace gol