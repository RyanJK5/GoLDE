#include <cstdint>
#include <expected>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <string>
#include <variant>

#include "GameGrid.h"
#include "Graphics2D.h"
#include "RLEEncoder.h"

template<class... Ts> struct Overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;

std::string gol::RLEEncoder::EncodeRegion(const GameGrid& grid, const Rect& region, Vec2 offset)
{
	auto toString = [](const auto& vec)
	{
		return std::string{reinterpret_cast<const char*>(vec.data())};
	};

	const auto encodeType = RLEEncoder::SelectStorageType(static_cast<int64_t>(region.Width) * region.Height);
	
	return std::visit(Overloaded
	{
		[&](uint8_t)  { return toString(EncodeRegion<uint8_t >(grid, region, offset)); },
		[&](uint16_t) { return toString(EncodeRegion<uint16_t>(grid, region, offset)); },
		[&](uint32_t) { return toString(EncodeRegion<uint32_t>(grid, region, offset)); },
		[&](uint64_t) { return toString(EncodeRegion<uint64_t>(grid, region, offset)); },
	}, encodeType);
}

std::expected<gol::RLEEncoder::DecodeResult, uint32_t> gol::RLEEncoder::DecodeRegion(const char* data, uint32_t warnThreshold)
{
	const auto result8 =  DecodeRegion<uint8_t>(data, static_cast<uint8_t>(warnThreshold));
	if (result8 || result8.error() != std::numeric_limits<uint8_t>::max())
		return result8;

	const auto result16 = DecodeRegion<uint16_t>(data, static_cast<uint16_t>(warnThreshold));
	if (result16 || result16.error() != std::numeric_limits<uint16_t>::max())
		return result16;

	const auto result32 = DecodeRegion<uint32_t>(data, warnThreshold);
	if (result32 || result32.error() != std::numeric_limits<uint32_t>::max())
		return result32;

	const auto result64 = DecodeRegion<uint64_t>(data, warnThreshold);
	if (result64)
		return *result64;

	return std::unexpected{ result32.error() };
}

bool gol::RLEEncoder::WriteRegion(const GameGrid& grid, const Rect& region, 
		const std::filesystem::path& filePath, Vec2 offset)
{
	auto out = std::ofstream { filePath };
	if (!out.is_open())
		return false;

	const auto encodedData = EncodeRegion(grid, region, offset);
	out << encodedData;

	return true;
}

std::expected<gol::RLEEncoder::DecodeResult, std::string> gol::RLEEncoder::ReadRegion(const std::filesystem::path& filePath)
{
	auto in = std::ifstream { filePath };
	if (!in.is_open())
		return std::unexpected { "Failed to open file for reading." };

	const std::string data { std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>() };

	const auto decodeResult = RLEEncoder::DecodeRegion(data.c_str(), std::numeric_limits<uint32_t>::max());
	if (!decodeResult)
		return std::unexpected { "File contains too many cells." };
	return *decodeResult;
}



constexpr std::variant<uint8_t, uint16_t, uint32_t, uint64_t> gol::RLEEncoder::SelectStorageType(uint64_t count)
{
	if (count <= (std::numeric_limits<uint8_t>::max() >> 2))
		return uint8_t {};
	else if (count <= (std::numeric_limits<uint16_t>::max() >> 4))
		return uint16_t {};
	else if (count <= (std::numeric_limits<uint32_t>::max() >> 8))
		return uint32_t {};
	return uint64_t {};
}