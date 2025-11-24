#ifndef __ExecutionWidget_h__
#define __ExecutionWidget_h__

#include <imgui/imgui.h>
#include <font-awesome/IconsFontAwesome7.h>
#include <span>
#include <unordered_map>
#include <vector>

#include "ActionButton.h"
#include "KeyShortcut.h"
#include "GameEnums.h"
#include "Graphics2D.h"
#include "SimulationControlResult.h"

namespace gol
{
    class StartButton : public MultiActionButton<GameAction, true>
    {
    public:
        StartButton(const std::unordered_map<ActionVariant, std::vector<ImGuiKeyChord>>& shortcuts)
            : MultiActionButton({
				{ GameAction::Start,  shortcuts.at(GameAction::Start)  | KeyShortcut::MapChordsToVector },
                { GameAction::Pause,  shortcuts.at(GameAction::Pause)  | KeyShortcut::MapChordsToVector },
                { GameAction::Resume, shortcuts.at(GameAction::Resume) | KeyShortcut::MapChordsToVector }
            })
        {}
    protected:
        virtual Size2F Dimensions() const override final;
        virtual GameAction Action(GameState state) const override final;
        virtual std::string Label(GameState state) const override final;
        virtual bool Enabled(GameState state) const override final;
    };

    class ClearButton : public ActionButton<GameAction, false>
    {
    public:
        ClearButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButton(GameAction::Clear, shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const override final;
        virtual std::string Label(GameState state) const override final;
        virtual bool Enabled(GameState state) const override final;
    };

    class ResetButton : public ActionButton<GameAction, false>
    {
    public:
        ResetButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButton(GameAction::Reset, shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const override final;
        virtual std::string Label(GameState state) const override final;
        virtual bool Enabled(GameState state) const override final;
    };

    class RestartButton : public ActionButton<GameAction, true>
    {
    public:
        RestartButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButton(GameAction::Restart, shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const override final;
        virtual std::string Label(GameState state) const override final;
        virtual bool Enabled(GameState state) const override final;
    };

	class ExecutionWidget
	{
	public:
        ExecutionWidget() = default;

        ExecutionWidget(const std::unordered_map<ActionVariant, std::vector<ImGuiKeyChord>>& shortcuts)
            : m_StartButton   (shortcuts)
            , m_ClearButton   (shortcuts.at(GameAction::Clear   ))
            , m_ResetButton   (shortcuts.at(GameAction::Reset   ))
            , m_RestartButton (shortcuts.at(GameAction::Restart ))
        { }

        SimulationControlResult Update(GameState state);
	private:
        StartButton m_StartButton;
        ClearButton m_ClearButton;
        ResetButton m_ResetButton;
        RestartButton m_RestartButton;
	};
}

#endif