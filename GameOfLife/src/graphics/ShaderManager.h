#ifndef __ShaderManager_h__
#define __ShaderManager_h__

#include <cstdint>
#include <filesystem>
#include <glm/glm.hpp>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <utility>

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

        void AttachUniformVec4(std::string_view label, const glm::vec4& vec);
        void AttachUniformMatrix4(std::string_view label, const glm::mat4& matrix);
    private:
        uint32_t CompileShader(uint32_t type, std::string_view source) const;

        std::optional<IDPair> ParseShader(const std::filesystem::path& filePath) const;

        void CreateShader(uint32_t program, uint32_t shaderId);

        int32_t UniformLocation(std::string_view label);

        void Destroy();
    private:
        uint32_t m_ProgramID;
        std::unordered_map<std::string_view, int32_t> m_Uniforms;
    };
}

#endif