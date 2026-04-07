#ifndef HashLife_hpp_
#define HashLife_hpp_

#include <concepts>

#include "HashQuadtree.hpp"
#include "LifeAlgorithm.hpp"

namespace gol {

template <typename T>
concept HashDataStructure =
    std::derived_from<T, LifeDataStructure> &&
    requires(T obj, int32_t advanceDepth, std::stop_token stopToken) {
        { obj.Advance(advanceDepth, stopToken) } -> std::same_as<int32_t>;
    };

template <HashDataStructure DataStructure = HashQuadtree>
class HashLife : public LifeAlgorithm,
                 AlgorithmRegistrator<HashLife<DataStructure>> {
  public:
    static std::string_view Identifier;

    HashLife();

    BigInt Step(LifeDataStructure& data, const BigInt& numSteps,
                std::stop_token stopToken = {}) override;
};

template <HashDataStructure DataStructure>
std::string_view HashLife<DataStructure>::Identifier = "HashLife";

template <HashDataStructure DataStructure>
HashLife<DataStructure>::HashLife() : LifeAlgorithm(this) {}

// Returns the exponent of the greatest power of two less than `stepSize`.
constexpr inline int32_t Log2MaxAdvanceOf(const BigInt& stepSize) {
    if (stepSize.is_zero())
        return 0;

    return static_cast<int32_t>(boost::multiprecision::msb(stepSize));
}

constexpr inline BigInt BigPow2(int32_t exponent) { return BigOne << exponent; }

template <HashDataStructure DataStructure>
BigInt HashLife<DataStructure>::Step(LifeDataStructure& data,
                                     const BigInt& numSteps,
                                     std::stop_token stopToken) {
    auto& hashQuadtree = dynamic_cast<DataStructure&>(data);

    if (numSteps.is_zero()) // Hyper speed
        return BigPow2(hashQuadtree.Advance(-1, stopToken));

    BigInt generation{};
    while (generation < numSteps) {
        const auto advanceLevel = Log2MaxAdvanceOf(numSteps - generation);

        const auto gens =
            BigPow2(hashQuadtree.Advance(advanceLevel, stopToken));
        if (stopToken.stop_requested())
            return generation;
        generation += gens;
    }

    return generation;
}
} // namespace gol

#endif