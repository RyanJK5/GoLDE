#include <gtest/gtest.h>
#include <print>

#include "Graphics2D.h"
#include "RLEEncoder.h"

namespace gol
{
	constexpr static bool EncodeDecodeTest(std::integral auto value)
	{
		using namespace RLEEncoder;
		
		const auto encoded = EncodeNumber(value);
		const auto decoded = DecodeNumber(std::string_view{ encoded.data(), encoded.size() });

		return (decoded == value);
	}

	TEST(EncodeTest, EncodeDecode)
	{
		static_assert(EncodeDecodeTest(0));
		static_assert(EncodeDecodeTest(150));
		static_assert(EncodeDecodeTest(124987543));
		static_assert(EncodeDecodeTest(99999999999999));
		
		static_assert([] {
			for (auto i = 1LL; i < std::numeric_limits<int32_t>::max(); i <<= 2)
				if (!EncodeDecodeTest(i))
					return false;
			return true;
		}());

		static_assert(EncodeDecodeTest(static_cast<int64_t>(std::numeric_limits<uint64_t>::max() >> 16)));
		// SHOULD NOT COMPILE
		 //static_assert(EncodeDecodeTest(std::numeric_limits<uint64_t>::max() >> 15));
	}

	static std::expected<RLEEncoder::DecodeResult, std::string> EncodeDecodeRegionTest(const GameGrid& grid, Rect region, Vec2 offset)
	{
		const auto encoded = RLEEncoder::EncodeRegion(grid, region, offset);
		const auto decodeResult = RLEEncoder::DecodeRegion(encoded, 1000000);

		if (!decodeResult.has_value())
		{
			const auto str = std::format("Decode failed with error: {}",
				decodeResult.error().has_value()
				? std::format("Data contains too many cells ({})", decodeResult.error().value())
				: "Data is not in a valid format");
			return std::unexpected{ str };
		}

		std::println("   {}\n-> {}\n-> {}", grid.Data(), encoded, decodeResult->Grid.Data());

		return *decodeResult;
	}

	TEST(EncodeTest, SquareTest)
	{
		const LifeHashSet data{ {0, 0}, {1, 1}, {0, 1}, {1, 0} };
		GameGrid grid{};
		for (const auto pos : data)
			grid.Set(pos.X, pos.Y, true);

		constexpr static Vec2 offset{ 2, 2 };
		const auto result = EncodeDecodeRegionTest(grid, Rect{ 0, 0, 4, 4 }, offset);

		ASSERT_TRUE(result.has_value()) << result.error();

		EXPECT_EQ(result->Grid.Data(), grid.Data());
		EXPECT_EQ(result->Offset, offset);
	}

	TEST(EncodeTest, SingleCellTest)
	{
		GameGrid grid{};
		grid.Set(3, 4, true);

		constexpr static Vec2 offset{ 5, -3 };
		const auto result = EncodeDecodeRegionTest(grid, Rect{ 0, 0, 8, 8 }, offset);

		ASSERT_TRUE(result.has_value()) << result.error();

		EXPECT_EQ(result->Grid.Data(), grid.Data());
		EXPECT_EQ(result->Offset, offset);
	}

	TEST(EncodeTest, HorizontalLineTest)
	{
		const LifeHashSet data{ {0, 2}, {1, 2}, {2, 2}, {3, 2}, {4, 2} };
		GameGrid grid{};
		for (const auto pos : data)
			grid.Set(pos.X, pos.Y, true);

		constexpr static Vec2 offset{ -4, 7 };
		const auto result = EncodeDecodeRegionTest(grid, Rect{ 0, 0, 6, 6 }, offset);

		ASSERT_TRUE(result.has_value()) << result.error();

		EXPECT_EQ(result->Grid.Data(), grid.Data());
		EXPECT_EQ(result->Offset, offset);
	}

	TEST(EncodeTest, SparsePatternTest)
	{
		const LifeHashSet data{
			{0, 0}, {2, 5}, {3, 1}, {6, 6}, {8, 2}, {9, 9}
		};

		GameGrid grid{};
		for (const auto pos : data)
			grid.Set(pos.X, pos.Y, true);

		constexpr static Vec2 offset{ -10, -10 };
		const auto result = EncodeDecodeRegionTest(grid, Rect{ 0, 0, 10, 10 }, offset);

		ASSERT_TRUE(result.has_value()) << result.error();

		EXPECT_EQ(result->Grid.Data(), grid.Data());
		EXPECT_EQ(result->Offset, offset);
	}

	TEST(EncodeTest, EmptyRegionTest)
	{
		GameGrid grid{};

		constexpr static Vec2 offset{ 2, 2 };
		const auto result = EncodeDecodeRegionTest(grid, Rect{ 0, 0, 5, 5 }, offset);

		ASSERT_TRUE(result.has_value()) << result.error();

		EXPECT_TRUE(result->Grid.Data().empty());
		EXPECT_EQ(result->Offset, offset);
	}

	TEST(EncodeTest, IgnoresCellsOutsideRegionTest)
	{
		const LifeHashSet data{
			{-1, 0}, {0, 0}, {3, 3}, {4, 0}, {0, 4}, {8, 8}
		};

		GameGrid grid{};
		for (const auto pos : data)
			grid.Set(pos.X, pos.Y, true);

		GameGrid expected{};
		expected.Set(0, 0, true);
		expected.Set(3, 3, true);

		constexpr static Vec2 offset{ 1, 1 };
		const auto result = EncodeDecodeRegionTest(grid, Rect{ 0, 0, 4, 4 }, offset);

		ASSERT_TRUE(result.has_value()) << result.error();

		EXPECT_EQ(result->Grid.Data(), expected.Data());
		EXPECT_EQ(result->Offset, offset);
	}
}