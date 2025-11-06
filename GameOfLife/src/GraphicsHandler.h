#ifndef __DrawManager_h__
#define __DrawManager_h__

#include <vector>
#include <filesystem>

#include "Camera.h"
#include "Graphics2D.h"
#include "ShaderManager.h"
#include "Logging.h"

namespace gol
{
	struct GraphicsHandlerArgs
	{
		Rect ViewportBounds;
		Size2 GridSize;
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

		void DrawGrid(const std::set<Vec2>& grid, const GraphicsHandlerArgs& info);
		void DrawSelection(Vec2 gridPos, const GraphicsHandlerArgs& info);
		void ClearBackground(const GraphicsHandlerArgs& args) const;

		void BindFrameBuffer() const;
		void UnbindFrameBuffer() const;

		inline uint32_t TextureID() const { return m_Texture; }
	private:
		std::vector<float> GenerateGLBuffer(const std::set<Vec2>& grid) const;

		RectF GridToScreenBounds(Vec2 pos, const GraphicsHandlerArgs& info) const;

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