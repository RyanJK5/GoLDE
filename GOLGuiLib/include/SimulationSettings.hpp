#ifndef SimulationSettings_hpp_
#define SimulationSettings_hpp_

#include <cstdint>

#include "LifeAlgorithm.hpp"

namespace gol {

struct SimulationSettings {
    int64_t StepCount = 1;
    LifeAlgorithm Algorithm = LifeAlgorithm::HashLife;
    int32_t TickDelayMs = 1;
    bool HyperSpeed = false;
    bool GridLines = false;
};

} // namespace gol

#endif
