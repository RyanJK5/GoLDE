#ifndef Widget_h_
#define Widget_h_

#include <concepts>

#include "ActionButton.hpp"
#include "EditorResult.hpp"
#include "GameEnums.hpp"
#include "SimulationControlResult.hpp"

namespace gol {
class Widget;

class Widget {
  public:
    SimulationControlResult Update(this auto &&self,
                                   const EditorResult &state) {
        return self.UpdateImpl(state);
    }

  protected:
    template <ActionType ActType>
    inline static void UpdateResult(SimulationControlResult &result,
                                    const ActionButtonResult<ActType> &update) {
        if (!result.Action)
            result.Action = update.Action;
        if (!result.FromShortcut)
            result.FromShortcut = update.FromShortcut;
    }
};
} // namespace gol

#endif