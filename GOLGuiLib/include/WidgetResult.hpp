#ifndef WidgetResult_hpp_
#define WidgetResult_hpp_

#include <optional>

#include "SimulationCommand.hpp"

namespace gol {

struct WidgetResult {
    std::optional<SimulationCommand> Command{};
    bool FromShortcut = false;
};

} // namespace gol

#endif
