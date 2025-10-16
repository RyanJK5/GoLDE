#include <fstream>

#include "ShaderManager.h"
#include "Logging.h"
#include "GLException.h"

gol::ShaderManager::ShaderManager(const std::filesystem::path& shaderFilePath)
{
    GL_DEBUG(m_programID = glCreateProgram());
    try
    {
        std::optional<IDPair> shaderIds = ParseShader(shaderFilePath);

        if (!shaderIds)
            throw GLException(std::format("File '{}' could not be read", shaderFilePath.string()));

        CreateShader(m_programID, shaderIds.value().first);
        CreateShader(m_programID, shaderIds.value().second);

        GL_DEBUG(glLinkProgram(m_programID));
        GL_DEBUG(glValidateProgram(m_programID));
    }
    catch (GLException e)
    {
        GL_DEBUG(glDeleteProgram(m_programID));
        throw e;
    }
}

gol::ShaderManager::ShaderManager(ShaderManager&& other) noexcept
{
    m_programID = std::exchange(other.m_programID, 0);
}

gol::ShaderManager& gol::ShaderManager::operator=(ShaderManager&& other) noexcept
{
    if (this != &other)
    {
        Destroy();
        m_programID = std::exchange(other.m_programID, 0);
    }
    return *this;
}

gol::ShaderManager::~ShaderManager()
{
    Destroy();
}

void gol::ShaderManager::Destroy() { GL_DEBUG(glDeleteProgram(m_programID)); }

uint32_t gol::ShaderManager::Program() const
{
	return m_programID;
}

uint32_t gol::ShaderManager::CompileShader(uint32_t type, std::string_view source) const
{
    uint32_t id = glCreateShader(type);
    const char* src = source.data();

    GL_DEBUG(glShaderSource(id, 1, &src, nullptr));
    GL_DEBUG(glCompileShader(id));

    int result;
    GL_DEBUG(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result != GL_FALSE)
    {
        return id;
    }

    int length;
    GL_DEBUG(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
    char* message = new char[length];
    GL_DEBUG(glGetShaderInfoLog(id, length, &length, message));

    std::string error = "Failed to compile " + std::string(type == GL_VERTEX_SHADER ? "vertex" : "fragment") + " shader\n" + message;
    delete[] message;

    throw GLException(error);
}

std::optional<gol::ShaderManager::IDPair> gol::ShaderManager::ParseShader(const std::filesystem::path& shaderFilePath) const
{
    std::optional<uint32_t> id1{};
    std::optional<uint32_t> id2{};

    std::ifstream stream(shaderFilePath);
    if (!stream.is_open())
        return std::nullopt;

    std::string line;
    std::string shader;

    std::optional<uint32_t> type{};
    auto setType = [] (const std::string& line, std::optional<uint32_t>& type)
    {
        size_t spaceIndex = line.find(' ');
        std::string typeStr = line.substr(spaceIndex + 1, line.length() - spaceIndex);
        
        if (typeStr == "vertex")
            type = GL_VERTEX_SHADER;
        else if (typeStr == "fragment")
            type = GL_FRAGMENT_SHADER;
        else
            type = std::nullopt;
    };

    while (getline(stream, line))
    {
        if (line.find("#shader") == std::string::npos)
        {
            shader += line + '\n';
            continue;
        }

        if (!type)
        {
            setType(line, type);
            if (!type)
                break;
            continue;
        }

        id1 = CompileShader(type.value(), shader);

        shader = "";
        setType(line, type);
    }

    if (type)
        id2 = CompileShader(type.value(), shader);

    if (!id1 || !id2)
        return std::nullopt;

    return std::make_pair(id1.value(), id2.value());
}

void gol::ShaderManager::CreateShader(uint32_t program, uint32_t shaderId)
{
    GL_DEBUG(glAttachShader(program, shaderId));
    GL_DEBUG(glDeleteShader(shaderId));
}