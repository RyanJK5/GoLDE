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
			const std::filesystem::path& shaderDirectory, 
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
		void InitGridBuffer();

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

		ShaderManager m_GridShader;
		ShaderManager m_SelectionShader;
		
		GLVertexArray m_GridVAO;
		GLBuffer m_InstanceBuffer;
		GLBuffer m_CellBuffer;
		GLIndexBuffer m_CellIndexBuffer;

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
	std::vector<float> result {};
	result.reserve(grid.size() * 2);
	for (const Vec2& vec : grid)
	{
		result.push_back(vec.X + offset.X);
		result.push_back(vec.Y + offset.Y);
	}

	return result;
}

void gol::GraphicsHandler::DrawGrid(Vec2 offset, const std::ranges::input_range auto& grid, const GraphicsHandlerArgs& args)
{
	FrameBufferBinder binder { m_FrameBuffer };

	auto matrix = Camera.OrthographicProjection(args.ViewportBounds.Size());

	if (args.ShowGridLines && offset.X == 0 && offset.Y == 0)
	{
		GL_DEBUG(glUseProgram(m_SelectionShader.Program()));
		m_SelectionShader.AttachUniformMatrix4("u_MVP", matrix);
		m_SelectionShader.AttachUniformVec4("u_Color", { 0.2f, 0.2f, 0.2f, 1.f });
		DrawGridLines(offset, args);
	}

	GL_DEBUG(glUseProgram(m_GridShader.Program()));
	GL_DEBUG(glBindVertexArray(m_GridVAO.ID()));
	m_GridShader.AttachUniformVec2("u_CellSize", { args.CellSize.Width, args.CellSize.Height });
	m_GridShader.AttachUniformMatrix4("u_MVP", matrix);
	m_GridShader.AttachUniformVec4("u_Color", { 1.f, 1.f, 1.f, 1.f });

	auto positions = GenerateGLBuffer(offset, grid, args);

	GL_DEBUG(glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBuffer.ID()));
	
	GL_DEBUG(glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), nullptr, GL_DYNAMIC_DRAW));
	GL_DEBUG(glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(float), positions.data()));

	GL_DEBUG(glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(positions.size() / 2)));

	GL_DEBUG(glBindVertexArray(0));
}

#endif