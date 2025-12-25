#ifndef __DrawManager_h__
#define __DrawManager_h__

#include <cstdint>
#include <filesystem>
#include <memory>
#include <unordered_set>
#include <vector>

#include "Camera.h"
#include "GameGrid.h"
#include "GLBuffer.h"
#include "Graphics2D.h"
#include "ShaderManager.h"

namespace gol
{
	struct GraphicsHandlerArgs
	{
		Rect ViewportBounds;
		Size2 GridSize;
		Size2F CellSize;
		bool ShowGridLines = false;
	};

	class GraphicsHandler
	{
	public:
		Camera Camera;
	public:
		GraphicsHandler(
			const std::filesystem::path& shaderFilePath, 
			int32_t windowWidth, int32_t windowHeight,
			Color bgColor
		);

		GraphicsHandler(const GraphicsHandler& other) = delete;
		GraphicsHandler& operator=(const GraphicsHandler& other) = delete;

		GraphicsHandler(GraphicsHandler&& other) noexcept = default;
		GraphicsHandler& operator=(GraphicsHandler&& other) noexcept = default;
		~GraphicsHandler() = default;

		void RescaleFrameBuffer(const Rect& windowBounds, const Rect& viewportBounds);

		void DrawGrid(Vec2 offset, const LifeHashSet& grid, const GraphicsHandlerArgs& args);
		void DrawSelection(const Rect& region, const GraphicsHandlerArgs& info);
		void ClearBackground(const GraphicsHandlerArgs& args);

		void CenterCamera(const GraphicsHandlerArgs& viewportBounds);

		uint32_t TextureID() const { return m_Texture.ID(); }
	private:
		std::vector<float> GenerateGLBuffer(Vec2 offset, const LifeHashSet& grid, const GraphicsHandlerArgs& args) const;

		RectDouble GridToScreenBounds(const Rect& region, const GraphicsHandlerArgs& args) const;
	private:
		struct GridLineInfo
		{
			Vec2F UpperLeft;
			Vec2F LowerRight;
			Size2 GridSize;
		};
		GridLineInfo CalculateGridLineInfo(Vec2 offset, const GraphicsHandlerArgs& args) const;
		
		void DrawGridLines(Vec2 offset, const GraphicsHandlerArgs& args);
	private:
		Color m_BgColor;

		ShaderManager m_Shader;
		
		GLBuffer m_GridBuffer;
		GLBuffer m_GridLineBuffer;
		GLBuffer m_SelectionBuffer;
		GLIndexBuffer m_SelectionIndexBuffer;

		GLFrameBuffer m_FrameBuffer;
		GLTexture m_Texture;
		GLRenderBuffer m_renderBuffer;
	};
}

#endif