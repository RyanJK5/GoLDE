#ifndef __FileWidget_h__
#define __FileWidget_h__

#include <filesystem>
#include <imgui/imgui.h>
#include <string>
#include <span>

#include "ActionButton.h"
#include "ErrorWindow.h"
#include "GameEnums.h"
#include "Graphics2D.h"
#include "SimulationControlResult.h"
#include "Widget.h"

namespace gol
{
	class NewFileButton : public ActionButton<EditorAction, true>
	{
	public:
		NewFileButton(std::span<const ImGuiKeyChord> shortcuts);
	protected:
		virtual Size2F Dimensions() const override final;
		virtual std::string Label(const EditorResult& state) const override final;
		virtual bool Enabled(const EditorResult& state) const override final;
	};

	class UpdateFileButton : public ActionButton<EditorAction, false>
	{
	public:
		UpdateFileButton(std::span<const ImGuiKeyChord> shortcuts);
	protected:
		virtual Size2F Dimensions() const override final;
		virtual std::string Label(const EditorResult& state) const override final;
		virtual bool Enabled(const EditorResult& state) const override final;
	};

	class SaveButton : public ActionButton<EditorAction, false>
	{
	public:
		SaveButton(std::span<const ImGuiKeyChord> shortcuts);
	protected:
		virtual Size2F Dimensions() const override final;
		virtual std::string Label(const EditorResult& state) const override final;
		virtual bool Enabled(const EditorResult& state) const override final;
	};

	class LoadButton : public ActionButton<EditorAction, false>
	{
	public:
		LoadButton(std::span<const ImGuiKeyChord> shortcuts);
	protected:
		virtual Size2F Dimensions() const override final;
		virtual std::string Label(const EditorResult& state) const override final;
		virtual bool Enabled(const EditorResult& state) const override final;
	};

	class FileWidget : public Widget
	{
	public:
		FileWidget(const std::unordered_map<ActionVariant, std::vector<ImGuiKeyChord>>& shortcutInfo);
		friend Widget;
	private:
		SimulationControlResult UpdateImpl(const EditorResult& state);
	private:
		NewFileButton m_NewFileButton;
		UpdateFileButton m_UpdateFileButton;
		SaveButton m_SaveButton;
		LoadButton m_LoadButton;

		ErrorWindow m_FileNotOpened;
	};
}

#endif