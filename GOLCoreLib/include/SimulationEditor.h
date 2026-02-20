#ifndef __SimulationEditor_h__
#define __SimulationEditor_h__

#include <cstdint>
#include <glm/glm.hpp>
#include <optional>

#include "EditorResult.h"
#include "ErrorWindow.h"
#include "GameEnums.h"
#include "GameGrid.h"
#include "GraphicsHandler.h"
#include "Graphics2D.h"
#include "PresetSelectionResult.h"
#include "SelectionManager.h"
#include "SimulationControlResult.h"
#include "SimulationWorker.h"
#include "VersionManager.h"
#include "WarnWindow.h"

namespace gol
{
	class SimulationEditor
	{
	public:
		static constexpr float DefaultCellWidth = 20.f;
		static constexpr float DefaultCellHeight = 20.f;
	public:
		SimulationEditor(uint32_t id, const std::filesystem::path& path, Size2 windowSize, Size2 gridSize);

		Rect WindowBounds() const;
		Rect ViewportBounds() const;
		const std::filesystem::path& CurrentFilePath() const { return m_CurrentFilePath; }

		EditorResult Update(std::optional<bool> activeOverride, const SimulationControlResult& controlArgs, const PresetSelectionResult& presetArgs);
		
		uint32_t EditorID() const { return m_EditorID; }
		bool IsSaved() const;
		bool operator==(const SimulationEditor& other) const;
	private:
		struct DisplayResult
		{
			bool Visible = false;
			bool Selected = false;
			bool Closing = false;
		};
	private:
		SimulationState SimulationUpdate(const GraphicsHandlerArgs& args);
		SimulationState PaintUpdate(const GraphicsHandlerArgs& args);
		SimulationState PauseUpdate(const GraphicsHandlerArgs& args);

		void DrawHashLifeData(const HashQuadtree& quadtree, const GraphicsHandlerArgs& args);

		SimulationState StartSimulation();
		void StopSimulation(bool stealGrid);

		void UpdateVersion(const SimulationControlResult& args);

		DisplayResult DisplaySimulation(bool grabFocus);

		SimulationState UpdateState(const SimulationControlResult& action);
		void PasteSelection();
		void LoadFile(const gol::SimulationControlResult& result);
		gol::SimulationState ResizeGrid(const gol::SimulationControlResult& result);
		void UpdateViewport();
		std::optional<Vec2> CursorGridPos();
		std::optional<Vec2> ConvertToGridPos(Vec2F screenPos);

		void UpdateMouseState(Vec2 gridPos);
		void FillCells();
		void UpdateDragState();
	private:
		static constexpr double DefaultTickDelayMs = 0.;
	private:
		uint32_t m_EditorID;
		std::filesystem::path m_CurrentFilePath;

		SimulationState m_State = SimulationState::Paint;

		GameGrid m_Grid;
		GameGrid m_InitialGrid;

		std::unique_ptr<SimulationWorker> m_Worker;

		SelectionManager m_SelectionManager;
		VersionManager m_VersionManager;

		GraphicsHandler m_Graphics;
		RectF m_WindowBounds;

		ErrorWindow m_FileErrorWindow;
		WarnWindow m_PasteWarning;
		
		bool m_StopStepCommand = false;
		bool m_TakeKeyboardInput = false;
		bool m_TakeMouseInput = false;

		Vec2F m_LeftDeltaLast;
		Vec2F m_RightDeltaLast;
		
		EditorMode m_EditorMode = EditorMode::None;
	};
}

#endif