#include <GL/glew.h>
#include <array>
#include <source_location>
#include <string>
#include <string_view>

#include "Logging.h"

constexpr std::array<std::string_view, 4> TermBlacklist = {
    "class", "__cdecl", "__thiscall", "std::"};
constexpr std::array<std::string_view, 1> TemplateBlacklist = {"basic_string"};

static bool SkipAngleBrackets(
    const std::string &expression, size_t &pos,
    const std::array<std::string_view, TemplateBlacklist.size()> &skip) {
    if (expression[pos] != '<')
        return false;

    bool inSkip = false;
    for (auto &token : skip) {
        size_t tokenBegin = pos - token.size();
        if (expression.find(token, tokenBegin) == tokenBegin) {
            inSkip = true;
            break;
        }
    }
    if (!inSkip)
        return false;

    int openAngles = 1;
    while (pos < expression.length()) {
        pos++;
        if (expression[pos] == '<')
            openAngles++;
        else if (expression[pos] == '>')
            openAngles--;

        if (openAngles == 0)
            break;
    }

    return true;
}

static bool
SkipTokens(const std::string &expression, size_t &pos,
           const std::array<std::string_view, TermBlacklist.size()> &skip) {
    for (auto &token : skip) {
        if (expression.find(token, pos) == pos) {
            pos += token.length();
            if (expression[pos] != ' ')
                pos--;
            return true;
        }
    }
    return false;
}

std::string gol::logimpl::SimplifyFileName(const std::string &fileName) {
    size_t nameStart = fileName.find_last_of('\\');
    if (nameStart > 0 && nameStart < fileName.size() - 1) {
        return fileName.substr(nameStart + 1);
    }
    return fileName;
}

std::string gol::logimpl::SimplifyFunctionName(const std::string &funcName) {
    std::string result = "";
    for (size_t i = 0; i < funcName.length(); i++) {
        if (SkipTokens(funcName, i, TermBlacklist))
            continue;
        if (SkipAngleBrackets(funcName, i, TemplateBlacklist))
            continue;

        result += funcName[i];
    }
    return result;
}

constexpr std::string gol::logimpl::StringRepresentation(gol::LogCode code) {
    switch (code) {
    case gol::LogCode::GLError:
        return "[GL ERROR]";
    case gol::LogCode::Error:
        return "[ERROR]";
    case gol::LogCode::Warning:
        return "[WARNING]";
    case gol::LogCode::Info:
        return "[INFO]";
    }
    return "";
}

void gol::LogGLErrors(const std::source_location &location) {
    while (GLenum error = glGetError()) {
        gol::Log(gol::LogCode::GLError, location, "Error Code {}", error);
    }
}