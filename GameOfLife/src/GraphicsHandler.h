#ifndef __DrawManager_h__
#define __DrawManager_h__

#include <vector>
#include <filesystem>

#include "Graphics2D.h"
#include "ShaderManager.h"

namespace gol
{
	class GraphicsHandler
	{
	public:
		GraphicsHandler(const std::filesystem::path& shaderFilePath, int32_t windowWidth, int32_t windowHeight);

		GraphicsHandler(const GraphicsHandler& other) = delete;
		GraphicsHandler& operator=(const GraphicsHandler& other) = delete;

		GraphicsHandler(GraphicsHandler&& other) noexcept;
		GraphicsHandler& operator=(GraphicsHandler&& other) noexcept;
		
		~GraphicsHandler();

		void RescaleFrameBuffer(Size2 windowSize);

		void DrawGrid(const std::vector<float>& positions) const;
		void DrawSelection(const RectF& bounds) const;
		void ClearBackground(const Rect& windowBounds, const Rect& viewportBounds) const;

		void BindFrameBuffer() const;
		void UnbindFrameBuffer() const;

		inline uint32_t TextureID() const { return m_Texture; }
	private:

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