#ifndef __FileDialog_h__
#define __FileDialog_h__

#include <expected>
#include <filesystem>
#include <string>

namespace gol {
enum class FileFailureType { Cancelled, Error };

struct FileDialogFailure {
  FileFailureType Type;
  std::string Message;
};

namespace FileDialog {
std::expected<std::filesystem::path, FileDialogFailure>
OpenFileDialog(const std::string &filters, const std::string &defaultPath);

std::expected<std::filesystem::path, FileDialogFailure>
SaveFileDialog(const std::string &filters, const std::string &defaultPath);

std::expected<std::filesystem::path, FileDialogFailure>
SelectFolderDialog(const std::string &defaultPath);
}; // namespace FileDialog
} // namespace gol

#endif