#ifndef __DrawManager_h__
#define __DrawManager_h__

#include <cstdint>
#include <filesystem>
#include <ranges>
#include <memory>
#include <unordered_set>
#include <vector>

#include "Camera.h"
#include "GLBuffer.h"
#include "Graphics2D.h"
#include "ShaderManager.h"
#include "Logging.h"

namespace gol
{
	struct GraphicsHandlerArgs
	{
		Rect ViewportBounds;
		Size2 GridSize;
		Size2F CellSize;
		bool ShowGridLines = false;
	};

	struct FrameBufferBinder
	{
		FrameBufferBinder(const gol::GLFrameBuffer& buffer);

		~FrameBufferBinder();

		FrameBufferBinder(const FrameBufferBinder&) = delete;
		auto& operator=(const FrameBufferBinder&) = delete;

		FrameBufferBinder(FrameBufferBinder&&) = delete;
		auto& operator=(const FrameBufferBinder&&) = delete;
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

		void DrawGrid(Vec2 offset, const std::ranges::input_range auto& grid, const GraphicsHandlerArgs& args);
		void DrawSelection(const Rect& region, const GraphicsHandlerArgs& info);
		void ClearBackground(const GraphicsHandlerArgs& args);

		void CenterCamera(const GraphicsHandlerArgs& viewportBounds);

		uint32_t TextureID() const { return m_Texture.ID(); }
	private:
		std::vector<float> GenerateGLBuffer(Vec2 offset, const std::ranges::input_range auto& grid, const GraphicsHandlerArgs& args) const;

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

std::vector<float> gol::GraphicsHandler::GenerateGLBuffer(Vec2 offset, const std::ranges::input_range auto& grid, const GraphicsHandlerArgs& args) const
{
	float width = args.CellSize.Width;
	float height = args.CellSize.Height;
	std::vector<float> result{};
	result.reserve(8 * grid.size());
	for (const Vec2& vec : grid)
	{
		float xCoord = (vec.X + offset.X) * width;
		float yCoord = (vec.Y + offset.Y) * height;

		result.push_back(xCoord);
		result.push_back(yCoord);

		result.push_back(xCoord);
		result.push_back(yCoord + height);

		result.push_back(xCoord + width);
		result.push_back(yCoord + height);

		result.push_back(xCoord + width);
		result.push_back(yCoord);
	}

	return result;
}

void gol::GraphicsHandler::DrawGrid(Vec2 offset, const std::ranges::input_range auto& grid, const GraphicsHandlerArgs& args)
{
	FrameBufferBinder binder{ m_FrameBuffer };

	auto matrix = Camera.OrthographicProjection(args.ViewportBounds.Size());
	m_Shader.AttachUniformMatrix4("u_MVP", matrix);

	if (args.ShowGridLines && offset.X == 0 && offset.Y == 0)
	{
		m_Shader.AttachUniformVec4("u_Color", { 0.2f, 0.2f, 0.2f, 1.f });
		DrawGridLines(offset, args);
	}

	m_Shader.AttachUniformVec4("u_Color", { 1.f, 1.f, 1.f, 1.f });
	auto positions = GenerateGLBuffer(offset, grid, args);

	GL_DEBUG(glBindBuffer(GL_ARRAY_BUFFER, m_GridBuffer.ID()));
	GL_DEBUG(glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_DYNAMIC_DRAW));

	GL_DEBUG(glEnableVertexAttribArray(0));
	GL_DEBUG(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));

	GL_DEBUG(glDrawArrays(GL_QUADS, 0, positions.size() / 2));
}

#endif