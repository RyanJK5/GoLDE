#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>
#include <optional>
#include <filesystem>

namespace gol
{
    
    class ShaderManager
    {
    using IDPair = typename std::pair<unsigned int, unsigned int>;

    public:
        explicit ShaderManager(const std::filesystem::path& shaderFilePath);

        ShaderManager(const ShaderManager& other) = delete;

        ShaderManager(ShaderManager&& other) noexcept;

        ShaderManager& operator=(const ShaderManager& other) = delete;

        ShaderManager& operator=(ShaderManager&& other) noexcept;

        ~ShaderManager();

        uint32_t Program() const;
    private:
        uint32_t CompileShader(uint32_t type, std::string_view source) const;

        std::optional<IDPair> ParseShader(const std::filesystem::path& filePath) const;

        void CreateShader(uint32_t program, uint32_t shaderId);

        void Destroy();
    private:
        unsigned int m_programID;
    };
}