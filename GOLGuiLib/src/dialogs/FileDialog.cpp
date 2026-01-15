#include <expected>
#include <filesystem>
#include <functional>
#include <string>
#include <nfd.h>

#include "FileDialog.h"

namespace gol {
	namespace
	{
		using NFDFunction = std::function<nfdresult_t(const nfdchar_t*, const nfdchar_t*, nfdchar_t**)>;
	
		std::expected<std::filesystem::path, FileDialogFailure> CallNFDFunction(NFDFunction nfdFunction, 
				const std::string& filters, const std::string& defaultPath)
		{
			nfdchar_t* outPath = nullptr;
			auto result = nfdFunction(filters.c_str(), defaultPath.c_str(), &outPath);
	
			if (result == NFD_OKAY)
			{
				auto ret = std::filesystem::path { outPath };
				auto extension = ret.extension().string();
				delete outPath;
				return ret;
			}
			else if (result == NFD_CANCEL)
			{
				return std::unexpected<FileDialogFailure> { {.Type = FileFailureType::Cancelled } };
			}
	
			auto error = std::string{ NFD_GetError() };
			return std::unexpected<FileDialogFailure> { {.Type = FileFailureType::Error, .Message = error } };
		}
	}
	
	std::expected<std::filesystem::path, FileDialogFailure> FileDialog::OpenFileDialog(
			const std::string& filters, const std::string& defaultPath)
	{
		auto result = CallNFDFunction(NFD_OpenDialog, filters, defaultPath);
		if (result && result->extension().string() != ".gol")
		{
			return std::unexpected<FileDialogFailure> { { 
				.Type = FileFailureType::Error, 
				.Message = "Invalid file type selected. Please select a .gol file." 
			} };
		}
		return result;
	}
	
	std::expected<std::filesystem::path, FileDialogFailure> FileDialog::SaveFileDialog(
			const std::string& filters, const std::string& defaultPath)
	{
		auto result = CallNFDFunction(NFD_SaveDialog, filters, defaultPath);
		if (result)
		{
			if (result->extension().empty())
				*result += ".gol";
			else if (result->extension().string() == ".")
				*result += "gol";
		}
		return result;
	}
	
	std::expected<std::filesystem::path, FileDialogFailure> FileDialog::SelectFolderDialog(
		const std::string& defaultPath)
	{
		constexpr static auto pickFolder = [](const nfdchar_t*, const nfdchar_t* defaultPath, nfdchar_t** outPath)
		{
			return NFD_PickFolder(defaultPath, outPath);
		};
	
		return CallNFDFunction(pickFolder, "", defaultPath);
	}
}