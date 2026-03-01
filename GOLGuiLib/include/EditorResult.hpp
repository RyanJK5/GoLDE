#ifndef EditorResult_h_
#define EditorResult_h_

#include <filesystem>

#include "GameEnums.hpp"
#include "LifeAlgorithm.hpp"

namespace gol {
struct EditorResult {
    std::filesystem::path CurrentFilePath{};
    SimulationState State = SimulationState::Paint;
    LifeAlgorithm Algorithm = LifeAlgorithm::SparseLife;
    bool Active = true;
    bool Closing = false;
    bool SelectionActive = false;
    bool UndosAvailable = false;
    bool RedosAvailable = false;
    bool HasUnsavedChanges = false;
};
} // namespace gol

#endif