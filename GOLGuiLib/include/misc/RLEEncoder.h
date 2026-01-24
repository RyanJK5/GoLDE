#ifndef __RLEEncoder_h__
#define __RLEEncoder_h__

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "GameGrid.h"
#include "Graphics2D.h"
#include "Logging.h"

namespace gol::RLEEncoder
{
	template <std::unsigned_integral StorageType>
	static constexpr StorageType FormatNumber(StorageType value)
	{
		constexpr StorageType getData   = 0b00111111;
		constexpr StorageType indicator = 0b01000000;
		StorageType result = 0b0;
		
		for (int8_t i = 0; i < sizeof(StorageType); i++)
		{
			result |= ((getData << (i * 6)) & value) << (i * 2);
			result |= indicator << (i * 8);
		}

		return result;
	}

	template <std::unsigned_integral StorageType>
	static constexpr std::vector<StorageType> FormatDimension(int32_t dim)
	{
		uint32_t formatted = FormatNumber<uint32_t>(dim);
		std::vector<StorageType> result;
		for (int32_t i = static_cast<int32_t>(sizeof(int32_t) - sizeof(StorageType)); 
				i >= 0; i -= static_cast<int32_t>(sizeof(StorageType)))
			result.push_back(static_cast<StorageType>(formatted >> (i * 8)));
		std::ranges::reverse(result);
		return result;
	}

	template <std::unsigned_integral StorageType>
	static constexpr StorageType ReadNumber(const char* value)
	{
		constexpr uint8_t getData = 0b00111111;
		StorageType result = static_cast<StorageType>(0b0);

		for (uint32_t i = 0; i < sizeof(StorageType); i++)
		{
			result |= static_cast<StorageType>(getData & static_cast<const uint8_t>(value[i])) << static_cast<StorageType>(i * 6);
		}
		return result;
	}

	template <std::unsigned_integral StorageType>
	inline std::vector<StorageType> EncodeRegion(const GameGrid& grid, const Rect& region, Vec2 offset)
	{
		constexpr static StorageType largestValue = std::numeric_limits<StorageType>::max() >> (2 * sizeof(StorageType));

		auto encoded = std::vector<StorageType> {};
			
		const auto offsetXDim = FormatDimension<StorageType>(std::abs(offset.X));
		encoded.insert(encoded.end(), offsetXDim.begin(), offsetXDim.end());
			
		const auto offsetYDim = FormatDimension<StorageType>(std::abs(offset.Y));
		encoded.insert(encoded.end(), offsetYDim.begin(), offsetYDim.end());
			
		const auto widthDim = FormatDimension<StorageType>(region.Width);
		encoded.insert(encoded.end(), widthDim.begin(), widthDim.end());
			
		const auto heightDim = FormatDimension<StorageType>(region.Height);
		encoded.insert(encoded.end(), heightDim.begin(), heightDim.end());
			
		encoded.push_back(FormatNumber<StorageType>('0'));
		encoded.push_back(FormatNumber<StorageType>(offset.X >= 0 ? '0' : '1'));
		encoded.push_back(FormatNumber<StorageType>(offset.Y >= 0 ? '0' : '1'));

		auto runStart = Vec2 { region.X, region.Y - 1 } - offset;
		bool running = false;
		bool first = true;
		for (auto pos : grid.SortedData())
		{
			if (!region.InBounds(pos))
				continue;

			pos -= offset;

			if (!running)
			{
				auto count = static_cast<StorageType>((region.Height * (pos.X - runStart.X) + pos.Y - runStart.Y) - 1);
				if (count > largestValue)
					throw std::invalid_argument("Specified region contains data that is too large");
				if (count > 0)
					encoded.push_back(FormatNumber<StorageType>(count));
				else if (first)
					encoded[4 * sizeof(uint32_t) / sizeof(StorageType)] = FormatNumber<StorageType>('1');
				running = true;
				runStart = pos;
			}

			first = false;
			auto nextPos = Vec2 { pos.X, pos.Y + 1 };
			if (nextPos.Y >= region.Y + region.Height - offset.Y)
			{
				nextPos.X++;
				nextPos.Y = region.Y - offset.Y;
			}

			if (!region.InBounds(nextPos + offset) || grid.Data().find(nextPos + offset) == grid.Data().end())
			{
				StorageType count = static_cast<StorageType>((region.Height * (pos.X - runStart.X) + pos.Y - runStart.Y) + 1);
				if (count > largestValue)
					throw std::invalid_argument("Specified region contains data that is too large");
				encoded.push_back(FormatNumber<StorageType>(count));
				running = false;
				runStart = pos;
			}
		}
		if (!running)
		{
			gol::Vec2 pos = region.LowerRight() - offset;
			StorageType count = static_cast<StorageType>((region.Height * (pos.X - 1 - runStart.X) + pos.Y - runStart.Y) - 1);
			if (count > largestValue)
			{
				ERROR("{} * {} = {} > {}", region.Width, region.Height, region.Width * region.Height, largestValue);
				throw std::invalid_argument("Specified region contains data that is too large");
			}
			encoded.push_back(FormatNumber<StorageType>(count));
		}

		if (encoded.size() > 2)
		{
			encoded.push_back('\0');
			return encoded;
		}
		return { '\0' };
	}

	struct DecodeResult
	{
		GameGrid Grid;
		Vec2 Offset;
	};

	template <std::unsigned_integral StorageType>
	inline std::expected<DecodeResult, StorageType> DecodeRegion(const char* data, StorageType warnThreshold)
	{
		if (data[0] == '\0')
			return {};

		auto offset = Vec2
		{
			static_cast<int32_t>(ReadNumber<uint32_t>(data)),
			static_cast<int32_t>(ReadNumber<uint32_t>(data + sizeof(uint32_t)))
		};

		auto width  = static_cast<int32_t>(ReadNumber<uint32_t>(data + sizeof(uint32_t) * 2));
		auto height = static_cast<int32_t>(ReadNumber<uint32_t>(data + sizeof(uint32_t) * 3));
		if (width * height > static_cast<int32_t>(std::numeric_limits<StorageType>::max() >> (2 * sizeof(StorageType))))
			return std::unexpected { std::numeric_limits<StorageType>::max() };

		bool running = ReadNumber<StorageType>(data + 4 * sizeof(uint32_t)) == '1';
		StorageType xPtr = 0;
		StorageType yPtr = 0;
		
		if (ReadNumber<StorageType>(data + sizeof(StorageType) + 4 * sizeof(uint32_t)) == '1')
			offset.X = -offset.X;
		if (ReadNumber<StorageType>(data + 2 * sizeof(StorageType) + 4 * sizeof(uint32_t)) == '1')
			offset.Y = -offset.Y;

		auto result = GameGrid { width, height };

		StorageType warnCount = 0;
		for (uint32_t i = 4 * sizeof(uint32_t) + 3 * sizeof(StorageType); data[i] != '\0'; i += sizeof(StorageType))
		{
			StorageType count = ReadNumber<StorageType>(data + i);
			if (running)
				warnCount += count;
			if (warnCount >= warnThreshold)
			{
				running = !running;
				continue;
			}

			if (running)
			{
				for (StorageType j = 0; j < count; j++)
				{
					result.Set(
						static_cast<int32_t>(xPtr + (yPtr + j) / height),
						static_cast<int32_t>((yPtr + j) % height),
						true
					);
				}
			}

			running = !running;
			xPtr += static_cast<StorageType>((yPtr + count) / height);
			yPtr = static_cast<StorageType>((yPtr + count) % height);
		}

		if (warnCount >= warnThreshold)
			return std::unexpected { warnCount };
		return DecodeResult { std::move(result), offset };
	}

	std::string EncodeRegion(const GameGrid& grid, const Rect& region, Vec2 offset = { 0, 0 } );

	std::expected<DecodeResult, uint32_t> DecodeRegion(const char* data, uint32_t warnThreshold);

	bool WriteRegion(const GameGrid& grid, const Rect& region, 
		const std::filesystem::path& filePath, Vec2 offset = { 0, 0 });

	std::expected<DecodeResult, std::string> ReadRegion(const std::filesystem::path& filePath);

	constexpr std::variant<uint8_t, uint16_t, uint32_t, uint64_t> SelectStorageType(uint64_t count);
}

#endif