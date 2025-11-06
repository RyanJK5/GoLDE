#ifndef __GameActionButton_h__
#define __GameActionButton_h__

#include <string_view>
#include <string>
#include <span>
#include <vector>
#include <functional>

#include "Graphics2D.h"
#include "GameEnums.h"
#include "KeyShortcut.h"

namespace gol
{
	class GameActionButton
	{
    public:
        constexpr static int32_t DefaultButtonHeight = 50;
	public:
		GameActionButton(
			std::string_view label, 
			GameAction actionReturn,
            std::span<const ImGuiKeyChord> shortcuts,
			bool lineBreak = false
		);

		GameAction Update(GameState state);
	protected:
		virtual Size2F Dimensions() const = 0;
		virtual	bool Enabled(GameState state) const = 0;
	private:
		std::string m_Label;
		GameAction m_Return;

		std::vector<KeyShortcut> m_Shortcuts;
		bool m_LineBreak;
	};

    namespace
    {
        template <size_t Length>
        struct StringLiteral
        {
            constexpr StringLiteral(const char(&str)[Length])
            {
                std::copy_n(str, Length, value);
            }

            char value[Length];
        };

        template <auto Label, GameAction Action, bool LineBreak>
        class HiddenTemplatedButton : public GameActionButton
        {
        public:
            HiddenTemplatedButton() = default;

            HiddenTemplatedButton(std::span<const ImGuiKeyChord> shortcuts)
                : GameActionButton(Label, Action, shortcuts, LineBreak)
            {}
        };

        template <StringLiteral Label, GameAction Action, bool LineBreak>
        using TemplatedButton = HiddenTemplatedButton<Label.value, Action, LineBreak>;
    }
}

#endif