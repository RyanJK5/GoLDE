#ifndef __RLEEncoder_h__
#define __RLEEncoder_h__

#include <concepts>

#include "GameGrid.h"
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>
#include "Graphics2D.h"

namespace gol::RLEEncoder
{
	
	template <std::integral StorageType>
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

	template <std::integral StorageType>
	static constexpr StorageType ReadNumber(const char* value)
	{
		constexpr int8_t getData = 0b00111111;
		StorageType result = 0b0;

		for (int8_t i = 0; i < sizeof(StorageType); i++)
		{
			result |= (getData & static_cast<const uint8_t>(value[i])) << (i * 6);
		}
		return result;
	}

	template <std::integral StorageType>
	inline std::vector<StorageType> EncodeRegion(const GameGrid& grid, const Rect& region)
	{
		constexpr StorageType largestValue = std::numeric_limits<StorageType>::max() >> (2 * sizeof(StorageType));

		std::vector<StorageType> encoded;
		encoded.emplace_back(FormatNumber<StorageType>(region.Width));
		encoded.emplace_back(FormatNumber<StorageType>(region.Height));
		encoded.emplace_back(FormatNumber<StorageType>('0'));

		gol::Vec2 runStart = { region.X, region.Y - 1 };
		bool running = false;
		bool first = true;
		for (const auto& pos : grid.Data())
		{
			if (!region.InBounds(pos))
				continue;

			if (!running)
			{
				StorageType count = (region.Height * (pos.X - runStart.X) + pos.Y - runStart.Y) - 1;
				if (count > largestValue)
					throw std::invalid_argument("Specified region contains data that is too large");
				if (count > 0)
					encoded.emplace_back(FormatNumber<StorageType>(count));
				else if (first)
					encoded[2] = FormatNumber<StorageType>('1');
				running = true;
				runStart = pos;
			}

			first = false;
			gol::Vec2 nextPos = { pos.X, pos.Y + 1 };
			if (nextPos.Y >= region.Y + region.Height)
			{
				nextPos.X++;
				nextPos.Y = region.Y;
			}

			if (!region.InBounds(nextPos) || grid.Data().find(nextPos) == grid.Data().end())
			{
				StorageType count = (region.Height * (pos.X - runStart.X) + pos.Y - runStart.Y) + 1;
				if (count > largestValue)
					throw std::invalid_argument("Specified region contains data that is too large");
				encoded.emplace_back(FormatNumber<StorageType>(count));
				running = false;
				runStart = pos;
			}
		}
		if (!running)
		{
			gol::Vec2 pos = region.LowerRight();
			StorageType count = (region.Height * (pos.X - 1 - runStart.X) + pos.Y - runStart.Y) - 1;
			if (count > largestValue)
				throw std::invalid_argument("Specified region contains data that is too large");
			encoded.push_back(FormatNumber<StorageType>(count));
		}

		if (encoded.size() > 2)
		{
			encoded.push_back('\0');
			return encoded;
		}
		return { '\0' };
	}

	template <std::integral StorageType>
	inline gol::GameGrid DecodeRegion(const char* data)
	{
		if (data[0] == '\0')
			return {};

		StorageType width = ReadNumber<StorageType>(data);
		StorageType height = ReadNumber<StorageType>(data + sizeof(StorageType));

		bool running = ReadNumber<StorageType>(data + 2 * sizeof(StorageType)) == '1';
		StorageType xPtr = 0;
		StorageType yPtr = 0;

		GameGrid result(width, height);

		for (size_t i = 3 * sizeof(StorageType); data[i] != '\0'; i += sizeof(StorageType))
		{
			StorageType count = ReadNumber<StorageType>(data + i);

			if (!running)
				running = true;
			else
			{
				for (StorageType j = 0; j < count; j++)
				{
					result.Set(
						xPtr + (yPtr + j) / height,
						(yPtr + j) % height,
						true
					);
				}
				running = false;
			}

			xPtr += (yPtr + count) / height;
			yPtr = (yPtr + count) % height;
		}

		return result;
	}
}

#endif