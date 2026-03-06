#ifndef StepWidget_h_
#define StepWidget_h_

#include <cstdint>
#include <imgui/imgui.h>
#include <span>
#include <string>

#include "ActionButton.hpp"
#include "EditorResult.hpp"
#include "GameEnums.hpp"
#include "Graphics2D.hpp"
#include "InputString.hpp"
#include "SimulationControlResult.hpp"
#include "Widget.hpp"

namespace gol {
class StepButton : public ActionButton<GameAction, false> {
  public:
    StepButton(std::span<const ImGuiKeyChord> shortcuts = {})
        : ActionButton(GameAction::Step, shortcuts, true) {}

  protected:
    virtual Size2F Dimensions() const override final;
    virtual std::string Label(const EditorResult &) const override final;
    virtual bool Enabled(const EditorResult &state) const override final;
};

class StepWidget : public Widget {
  public:
    static constexpr int32_t BigStep = 10;

  public:
    StepWidget(std::span<const ImGuiKeyChord> shortcuts = {})
        : m_InputText(
              "1",
              std::to_string(std::numeric_limits<uint64_t>::max()).length()),
          m_Button(shortcuts) {}

    friend Widget;

  private:
    SimulationControlResult UpdateImpl(const EditorResult &state);

    void SetShortcutsImpl(const ShortcutMap &) { }

    void SetStepCount(int64_t stepCount);

    void ShowInputText();

  private:
    InputString m_InputText;

    LifeAlgorithm m_Algorithm = LifeAlgorithm::HashLife;
    int64_t m_StepCount = 1;
    bool m_HyperSpeed = false;

    StepButton m_Button;
};
} // namespace gol

#endif