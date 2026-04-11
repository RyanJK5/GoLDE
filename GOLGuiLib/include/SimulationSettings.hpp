#ifndef SimulationSettings_hpp_
#define SimulationSettings_hpp_

#include <cstdint>

#include "LifeAlgorithm.hpp"

namespace gol {

struct SimulationSettings {
    BigInt StepCount = BigOne;
    std::unique_ptr<LifeAlgorithm> Algorithm = nullptr;
    int32_t TickDelayMs = 1;
    bool HyperSpeed = false;
    bool GridLines = false;
};

} // namespace gol

#endif
