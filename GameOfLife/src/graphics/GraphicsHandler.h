#ifndef __DrawManager_h__
#define __DrawManager_h__

#include <cstdint>
#include <filesystem>
#include <set>
#include <vector>

#include "Camera.h"
#include "Graphics2D.h"
#include "ShaderManager.h"

namespace gol
{
	struct GraphicsHandlerArgs
	{
		Rect ViewportBounds;
		Size2 GridSize;
		bool ShowGridLines = false;
	};

	class GraphicsHandler
	{
	public:
		Camera Camera;
	public:
		GraphicsHandler(const std::filesystem::path& shaderFilePath, int32_t windowWidth, int32_t windowHeight);

		GraphicsHandler(const GraphicsHandler& other) = delete;
		GraphicsHandler& operator=(const GraphicsHandler& other) = delete;

		GraphicsHandler(GraphicsHandler&& other) noexcept;
		GraphicsHandler& operator=(GraphicsHandler&& other) noexcept;
		
		~GraphicsHandler();

		void RescaleFrameBuffer(Size2 windowSize);

		void DrawGrid(Vec2 offset, const std::set<Vec2>& grid, const GraphicsHandlerArgs& info);
		void DrawSelection(const Rect& region, const GraphicsHandlerArgs& info);
		void ClearBackground(const GraphicsHandlerArgs& args) const;

		void BindFrameBuffer() const;
		void UnbindFrameBuffer() const;

		inline uint32_t TextureID() const { return m_Texture; }
	private:
		void DrawGridLines(Vec2 offfset, const GraphicsHandlerArgs& info);

		std::vector<float> GenerateGLBuffer(Vec2 offset, const std::set<Vec2>& grid) const;

		RectF GridToScreenBounds(const Rect& region, const GraphicsHandlerArgs& info) const;

		void Move(GraphicsHandler&& other) noexcept;
		void Destroy();
	private:
		ShaderManager m_Shader;
		
		uint32_t m_GridBuffer;
		uint32_t m_SelectionBuffer;
		uint32_t m_SelectionIndexBuffer;

		uint32_t m_FrameBuffer;
		uint32_t m_Texture;
		uint32_t m_renderBuffer;
	};
}

#endif