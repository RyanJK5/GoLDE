#include <gtest/gtest.h>
#include <algorithm>
#include <ranges>
#include <random>
#include <print>

#include "HashQuadtree.h"
#include "RLEEncoder.h"

namespace gol
{
    // Helper to verify the tree iterator yields exactly the expected points
    static void VerifyContent(HashQuadtree& tree, const LifeHashSet& expected)
    {
        LifeHashSet actual;

        // Test range-based for loop
        for (const auto& pos : tree)
        {
            actual.insert(pos);
        }

        ASSERT_EQ(actual.size(), expected.size()) << "Tree yielded wrong number of cells";

        for (const auto& cell : expected)
        {
            ASSERT_TRUE(actual.contains(cell)) << "Tree missing a cell that should be there";
        }
    }

	static void CheckAgainstFile(const std::filesystem::path& unevolved, const std::filesystem::path& evolved, uint64_t expectedGenerations)
    {
        ASSERT_TRUE(std::filesystem::exists(unevolved));
        ASSERT_TRUE(std::filesystem::exists(evolved));

        const auto data1 = RLEEncoder::ReadRegion(unevolved);
        const auto data2 = RLEEncoder::ReadRegion(evolved);
        const HashQuadtree tree1{ data1->Grid.Data(), data1->Offset };
        const HashQuadtree tree2{ data2->Grid.Data(), data2->Offset };

        const auto update = tree1.NextGeneration();
        EXPECT_EQ(update.Generations, expectedGenerations) << "HashLife should advance by" << expectedGenerations << " generations";
        EXPECT_EQ(update.Data, tree2);
    }

    TEST(HashQuadtreeTest, SquigglesTest)
    {
		const std::filesystem::path directory{ "universes" };
        CheckAgainstFile(directory / "squiggles1.gol", directory / "squiggles64.gol", 64);
    }

    TEST(HashQuadtreeTest, BigSquigglesTest)
    {
        const std::filesystem::path directory{ "universes" };
        CheckAgainstFile(directory / "bigsquiggles1.gol", directory / "bigsquiggles256.gol", 256);
    }

    TEST(HashQuadtreeTest, RectTest)
    {
        const std::filesystem::path directory{ "universes" };
		CheckAgainstFile(directory / "rect1.gol", directory / "rect4.gol", 4);
    }

    TEST(HashQuadtreeTest, EmptyTree)
    {
        LifeHashSet cells{};
        HashQuadtree tree{ cells };

        EXPECT_EQ(tree.begin(), tree.end()) << "Empty tree should only contain end iterator";
        EXPECT_TRUE(std::ranges::empty(tree)) << "Empty tree should satisfy std::ranges::empty";

        VerifyContent(tree, cells);
    }

    TEST(HashQuadtreeTest, SingleCell)
    {
        LifeHashSet cells = { {10, 20} };
        HashQuadtree tree{ cells };

        ASSERT_NE(tree.begin(), tree.end());
        EXPECT_EQ((*tree.begin()).X, 10);
        EXPECT_EQ((*tree.begin()).Y, 20);

        VerifyContent(tree, cells);
    }

    TEST(HashQuadtreeTest, BlockPattern)
    {
        // Standard 2x2 block
        LifeHashSet cells
        {
            {0, 0}, {1, 0},
            {0, 1}, {1, 1}
        };
        HashQuadtree tree{ cells };

        EXPECT_EQ(std::ranges::distance(tree), 4);
        VerifyContent(tree, cells);
    }

    TEST(HashQuadtreeTest, SparsePattern)
    {
        // Points far away to force deep quadtree recursion
        LifeHashSet cells = {
            { -1280, -1208 },
            { 0, 0 },
            { 1208, 1028 }
        };
        HashQuadtree tree{ cells };

        EXPECT_EQ(std::ranges::distance(tree), 3);
        VerifyContent(tree, cells);
    }

    TEST(HashQuadtreeTest, LargeDenseGrid)
    {
        LifeHashSet cells;
        for (int x = 0; x < 100; ++x)
        {
            for (int y = 0; y < 100; ++y)
            {
                cells.insert({ x, y });
            }
        }
        HashQuadtree tree{ cells };
        VerifyContent(tree, cells);
    }

    TEST(HashQuadtreeTest, DiagonalLine)
    {
        LifeHashSet cells;
        for (int i = 0; i < 1000; ++i)
        {
            cells.insert({ i, i });
        }
        HashQuadtree tree{ cells };
        VerifyContent(tree, cells);
    }

    TEST(HashQuadtreeTest, RandomSparse)
    {
        LifeHashSet cells;
        std::mt19937 gen{ 12345 }; // Fixed seed for reproducibility
        std::uniform_int_distribution<> dist{ -10000, 10000 };

        for (int i = 0; i < 500; ++i)
        {
            cells.insert({ dist(gen), dist(gen) });
        }
        HashQuadtree tree{ cells };
        VerifyContent(tree, cells);
    }

    TEST(HashQuadtreeTest, Checkerboard)
    {
        LifeHashSet cells;
        for (int x = 0; x < 50; ++x)
        {
            for (int y = 0; y < 50; ++y)
            {
                if ((x + y) % 2 == 0) cells.insert({ x, y });
            }
        }
        HashQuadtree tree{ cells };
        VerifyContent(tree, cells);
    }

    TEST(HashQuadtreeTest, RangesCompliance)
    {
        static_assert(std::ranges::range<HashQuadtree>);
        static_assert(std::ranges::input_range<HashQuadtree>);

        LifeHashSet cells = { {1, 1}, {2, 2}, {3, 3} };
        HashQuadtree tree{ cells };

        // Test with std::algorithms
        auto count = std::count_if(tree.begin(), tree.end(), [](const Vec2& v)
        {
            return v.X > 1; // Should find {2,2} and {3,3}
        });
        EXPECT_EQ(count, 2);
    }

    TEST(HashQuadtreeTest, AdvanceBlock)
    {
        // 2x2 Block (Still Life)
        LifeHashSet cells{ {0,0}, {1,0}, {0,1}, {1,1} };
        HashQuadtree tree{ cells };

        // Should remain stable
        auto update = tree.NextGeneration();
        EXPECT_GE(update.Generations, 2); // At least 2 generations (level 3)
        tree = update.Data;
        VerifyContent(tree, cells);

        // Advance again
        update = tree.NextGeneration();
        EXPECT_GE(update.Generations, 2);
        tree = update.Data;
        VerifyContent(tree, cells);
    }

    TEST(HashQuadtreeTest, AdvanceBlinker)
    {
        // Blinker (Period 2 Oscillator)
        LifeHashSet start{ {0,0}, {0,1}, {0,2} };
        HashQuadtree tree(start);

        auto update = tree.NextGeneration();
        // HashLife advances by 2^(k-2) steps. For k=3, steps=2.
        // A blinker's period is 2, so it should be back to start if steps is a multiple of 2.
        EXPECT_EQ(update.Generations % 2, 0) << "HashLife should advance by a multiple of blinker period";
        
        tree = update.Data;

        LifeHashSet actual;
        for (const auto& p : tree) actual.insert(p);

        // Blinker should still be vertical
        ASSERT_EQ(actual.size(), 3);
        EXPECT_TRUE(actual.contains({ 0,0 }));
        EXPECT_TRUE(actual.contains({ 0,1 }));
        EXPECT_TRUE(actual.contains({ 0,2 }));
    }

    TEST(HashQuadtreeTest, AdvanceGlider)
    {
        // Glider
        LifeHashSet start{
            {1, 0},
            {2, 1},
            {0, 2}, {1, 2}, {2, 2}
        };

        HashQuadtree tree{ start };
        auto update = tree.NextGeneration();
        EXPECT_GT(update.Generations, 0);

        LifeHashSet actual;
        for (const auto& p : update.Data) actual.insert(p);

        // Glider should survive (5 cells)
        EXPECT_EQ(actual.size(), 5);

        // And should have moved from original position
        bool sameAsStart = true;
        for (const auto& cell : start)
        {
            if (!actual.contains(cell))
            {
                sameAsStart = false;
                break;
            }
        }
        EXPECT_FALSE(sameAsStart) << "Glider did not move";
    }

    TEST(HashQuadtreeTest, CalculateLevelAndSize)
    {
        // Empty Tree
        {
            LifeHashSet cells{};
            HashQuadtree tree{ cells };
            // Empty tree is represented by FalseNode (Level 0, Size 1)
            EXPECT_EQ(tree.CalculateDepth(), 0);
            EXPECT_EQ(tree.CalculateTreeSize(), 0);
        }

        // Single Cell
        {
            LifeHashSet cells{ {0, 0} };
            HashQuadtree tree{ cells };
            // Single cell is TrueNode (Level 0, Size 1)
            EXPECT_EQ(tree.CalculateDepth(), 0);
            EXPECT_EQ(tree.CalculateTreeSize(), 1);
        }

        // 2x2 Block
        {
            LifeHashSet cells{ {0, 0}, {1, 0}, {0, 1}, {1, 1} };
            HashQuadtree tree{ cells };
            // 2x2 fits in Level 1 (Size 2)
            EXPECT_EQ(tree.CalculateDepth(), 1);
            EXPECT_EQ(tree.CalculateTreeSize(), 2);
        }

        // 4x4 Area
        {
            LifeHashSet cells{ {0, 0}, {3, 3} };
            HashQuadtree tree{ cells };
            // Bounds: 0..3 -> Size 4 -> Level 2
            EXPECT_EQ(tree.CalculateDepth(), 2);
            EXPECT_EQ(tree.CalculateTreeSize(), 4);
        }

        // Large Area
        {
            LifeHashSet cells{ {0, 0}, {100, 100} };
            HashQuadtree tree{ cells };
            // Size needs to cover 0 to 100 (span 101). Next power of 2 is 128.
            // 128 = 2^7 -> Level 7
            EXPECT_EQ(tree.CalculateTreeSize(), 128);
            EXPECT_EQ(tree.CalculateDepth(), 7);
        }
    }

    TEST(HashQuadtreeTest, ConstIteratorUsage)
    {
        const LifeHashSet cells{ {1, 1}, {5, 5} };
        const HashQuadtree tree{ cells };

        // Ensure we can iterate over a const tree using range-based for
        size_t count = 0;
        for ([[maybe_unused]] const auto& cell : tree) 
        {
            count++;
        }
        EXPECT_EQ(count, 2);

        // Explicitly check ConstIterator type
        static_assert(std::is_same_v<HashQuadtree::ConstIterator, decltype(tree.begin())>);
    }

    TEST(HashQuadtreeTest, TranslationInvariance)
    {
        // HashLife should behave the same regardless of where the pattern is in the grid
        const LifeHashSet blockAtOrigin{ {0, 0}, {1, 0}, {0, 1}, {1, 1} };
        const LifeHashSet blockAtDistance{ {1000, 1000}, {1001, 1000}, {1000, 1001}, {1001, 1001} };

        HashQuadtree tree1{ blockAtOrigin };
        HashQuadtree tree2{ blockAtDistance };

        auto next1 = tree1.NextGeneration();
        auto next2 = tree2.NextGeneration();

        EXPECT_EQ(next1.Generations, next2.Generations) << "Translation should not affect step size";
        EXPECT_EQ(std::ranges::distance(next1.Data), 4);
        EXPECT_EQ(std::ranges::distance(next2.Data), 4);
    }

    TEST(HashQuadtreeTest, UniverseHeatDeath)
    {
        // A single cell dies in the next generation
        LifeHashSet cells{ {42, 42} };
        HashQuadtree tree{ cells };

        const auto next = tree.NextGeneration();
        
        EXPECT_GT(next.Generations, 0);
        EXPECT_TRUE(next.Data.empty());
        EXPECT_EQ(next.Data.begin(), next.Data.end());
        EXPECT_EQ(std::ranges::distance(next.Data), 0);
    }

    TEST(HashQuadtreeTest, LargeCoordinateStability)
    {
        // Test with coordinates that might challenge bit-depth or offsets
        const int32_t far = 1'000'000;
        const LifeHashSet cells{ {far, far}, {far + 1, far}, {far, far + 1}, {far + 1, far + 1} };
        HashQuadtree tree{ cells };

        EXPECT_EQ(std::ranges::distance(tree), 4);
        
        auto update = tree.NextGeneration();
        EXPECT_GT(update.Generations, 0);
        tree = update.Data;
        
        EXPECT_EQ(std::ranges::distance(tree), 4);
        for (const auto& pos : tree)
        {
            EXPECT_GE(pos.X, far);
            EXPECT_GE(pos.Y, far);
        }
    }

    TEST(HashQuadtreeTest, LifecycleAndCopy)
    {
        // Testing that copy and move work correctly and maintain tree integrity
        LifeHashSet cells{ {1,1}, {2,2} };
        HashQuadtree tree1{ cells };
        
        // Copy construction
        HashQuadtree tree2 = tree1;
        VerifyContent(tree2, cells);
        
        // Move construction
        HashQuadtree tree3 = std::move(tree1);
        VerifyContent(tree3, cells);
        
        // Identity check via NextGeneration
        auto gen2 = tree2.NextGeneration();
        auto gen3 = tree3.NextGeneration();
        
        EXPECT_EQ(gen2.Generations, gen3.Generations);
        EXPECT_EQ(std::ranges::distance(gen2.Data), std::ranges::distance(gen3.Data));
    }

    TEST(HashQuadtreeTest, StandardViewComposition)
    {
        // Integration with C++23 views
        const LifeHashSet cells{ {0,0}, {1,1}, {2,2}, {10,10} };
        HashQuadtree tree{ cells };

        // Filter and count using ranges
        auto filtered = tree | std::views::filter([](const auto& p) { return p.X < 5; });
        EXPECT_EQ(std::ranges::distance(filtered), 3);

        // Sum coordinates
        const auto sumX = std::ranges::fold_left(tree | std::views::transform([](auto p) { return p.X; }), 0, std::plus<>{});
        EXPECT_EQ(sumX, 13);
    }
}