#ifndef __GLBuffer_h__
#define __GLBuffer_h__

#include <concepts>
#include <cstdint>
#include <utility>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Logging.h"

namespace gol
{
	template <typename T>
	concept GLGenerator = 
		std::same_as<T, decltype(&glGenBuffers)> ||
		std::same_as<T, decltype(&glGenTextures)>;

	template <typename T>
	concept GLDeleter = 
		std::same_as<T, decltype(glDeleteBuffers)*> ||
		std::same_as<T, decltype(glDeleteTextures)*>;

	template <
		GLGenerator GeneratorType, GLDeleter DeleterType, 
		GeneratorType Generator, DeleterType Deleter >
	class GLWrapper
	{
	public:
		GLWrapper();

		GLWrapper(const GLWrapper<GeneratorType, DeleterType, Generator, Deleter>&) = delete;
		
		auto& operator=(const GLWrapper<GeneratorType, DeleterType, Generator, Deleter>&) = delete;

		GLWrapper(GLWrapper<GeneratorType, DeleterType, Generator, Deleter>&& other) noexcept;
		
		auto& operator=(GLWrapper<GeneratorType, DeleterType, Generator, Deleter>&& other) noexcept;

		~GLWrapper();

		uint32_t ID() const { return m_ID; }
	private:
		uint32_t m_ID = 0;
	};

	using GLBuffer = GLWrapper<
		decltype(&glGenBuffers), decltype(&glDeleteBuffers), 
		&glGenBuffers, &glDeleteBuffers
	>;

	using GLIndexBuffer = GLBuffer;

	using GLFrameBuffer = GLWrapper<
		decltype(&glGenFramebuffers), decltype(&glDeleteFramebuffers),
		&glGenFramebuffers, &glDeleteFramebuffers
	>;

	using GLRenderBuffer = GLWrapper<
		decltype(&glGenRenderbuffers), decltype(&glDeleteRenderbuffers),
		&glGenRenderbuffers, &glDeleteRenderbuffers
	>;

	using GLTexture = GLWrapper<
		decltype(&glGenTextures), decltype(&glDeleteTextures),
		&glGenTextures, &glDeleteTextures
	>;
}

template <gol::GLGenerator GeneratorType, gol::GLDeleter DeleterType, GeneratorType Generator, DeleterType Deleter>
gol::GLWrapper<GeneratorType, DeleterType, Generator, Deleter>::GLWrapper()
{
	GL_DEBUG((*Generator)(1, &m_ID));
}

template <gol::GLGenerator GeneratorType, gol::GLDeleter DeleterType, GeneratorType Generator, DeleterType Deleter>
gol::GLWrapper<GeneratorType, DeleterType, Generator, Deleter>::GLWrapper(GLWrapper<GeneratorType, DeleterType, Generator, Deleter>&& other) noexcept
{
	m_ID = std::exchange(other.m_ID, 0);
}

template <gol::GLGenerator GeneratorType, gol::GLDeleter DeleterType, GeneratorType Generator, DeleterType Deleter>
auto& gol::GLWrapper<GeneratorType, DeleterType, Generator, Deleter>::operator=(GLWrapper<GeneratorType, DeleterType, Generator, Deleter>&& other) noexcept
{
	if (this != &other)
		m_ID = std::exchange(other.m_ID, 0);
	return *this;
}

template <gol::GLGenerator GeneratorType, gol::GLDeleter DeleterType, GeneratorType Generator, DeleterType Deleter>
gol::GLWrapper<GeneratorType, DeleterType, Generator, Deleter>::~GLWrapper()
{
	GL_DEBUG((*Deleter)(1, &m_ID));
}

#endif