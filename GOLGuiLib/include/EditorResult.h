#ifndef __EditorResult_h__
#define __EditorResult_h__

#include <filesystem>

#include "GameEnums.h"
#include "LifeAlgorithm.h"

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