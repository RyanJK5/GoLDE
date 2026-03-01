#ifndef ActionWidget_h_
#define ActionWidget_h_

#include <cstdint>
#include <filesystem>
#include <optional>

#include "GameEnums.hpp"
#include "Graphics2D.hpp"
#include "LifeAlgorithm.hpp"

namespace gol {
struct SimulationControlResult {
    SimulationState State = SimulationState::Empty;
    std::optional<ActionVariant> Action{};

    int64_t StepCount = 0;
    std::optional<LifeAlgorithm> Algorithm{};

    std::optional<Size2> NewDimensions{};

    std::optional<float> NoiseDensity{};

    std::optional<int32_t> TickDelayMs{};

    std::optional<std::filesystem::path> FilePath{};

    int32_t NudgeSize = 0;

    bool HyperSpeed = false;

    bool GridLines = false;

    bool FromShortcut = false;
};
} // namespace gol

#endif