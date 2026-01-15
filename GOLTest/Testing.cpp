#include <gtest/gtest.h>
#include <algorithm>
#include <ranges>
#include <random>

#include "HashQuadtree.h"

using namespace gol;

// Helper to verify the tree iterator yields exactly the expected points
static void VerifyContent(HashQuadtree& tree, const LifeHashSet& expected) {
    LifeHashSet actual;

    // Test range-based for loop
    for (const auto& pos : tree) {
        actual.insert(pos);
    }

    ASSERT_EQ(actual.size(), expected.size()) << "Tree yielded wrong number of cells";

    for (const auto& cell : expected) {
        ASSERT_TRUE(actual.contains(cell)) << "Tree missing a cell that should be there";
    }
}

TEST(HashQuadtreeTest, EmptyTree) {
    LifeHashSet cells {};
    HashQuadtree tree {cells};

    EXPECT_EQ(tree.begin(), tree.end()) << "Empty tree should only contain end iterator";
    EXPECT_TRUE(std::ranges::empty(tree)) << "Empty tree should satisfy std::ranges::empty";

    VerifyContent(tree, cells);
}

TEST(HashQuadtreeTest, SingleCell) {
    LifeHashSet cells = { {10, 20} };
    HashQuadtree tree { cells };

    ASSERT_NE(tree.begin(), tree.end());
    EXPECT_EQ((*tree.begin()).X, 10);
    EXPECT_EQ((*tree.begin()).Y, 20);

    VerifyContent(tree, cells);
}

TEST(HashQuadtreeTest, BlockPattern) {
    // Standard 2x2 block
    LifeHashSet cells
    {
        {0, 0}, {1, 0},
        {0, 1}, {1, 1}
    };
    HashQuadtree tree { cells };

    EXPECT_EQ(std::ranges::distance(tree), 4);
    VerifyContent(tree, cells);
}

TEST(HashQuadtreeTest, SparsePattern) {
    // Points far away to force deep quadtree recursion
    LifeHashSet cells = {
        { -1280, -1208 },
        { 0, 0 },
        { 1208, 1028 }
    };
    HashQuadtree tree { cells };

    EXPECT_EQ(std::ranges::distance(tree), 3);
    VerifyContent(tree, cells);
}

TEST(HashQuadtreeTest, LargeDenseGrid) {
    LifeHashSet cells;
    for (int x = 0; x < 100; ++x) {
        for (int y = 0; y < 100; ++y) {
            cells.insert({x, y});
        }
    }
    HashQuadtree tree(cells);
    VerifyContent(tree, cells);
}

TEST(HashQuadtreeTest, DiagonalLine) {
    LifeHashSet cells;
    for (int i = 0; i < 1000; ++i) {
        cells.insert({i, i});
    }
    HashQuadtree tree(cells);
    VerifyContent(tree, cells);
}

TEST(HashQuadtreeTest, RandomSparse) {
    LifeHashSet cells;
    std::mt19937 gen(12345); // Fixed seed for reproducibility
    std::uniform_int_distribution<> dist(-10000, 10000);

    for (int i = 0; i < 500; ++i) {
        cells.insert({dist(gen), dist(gen)});
    }
    HashQuadtree tree(cells);
    VerifyContent(tree, cells);
}

TEST(HashQuadtreeTest, Checkerboard) {
    LifeHashSet cells;
    for(int x = 0; x < 50; ++x) {
        for(int y = 0; y < 50; ++y) {
            if((x + y) % 2 == 0) cells.insert({x, y});
        }
    }
    HashQuadtree tree(cells);
    VerifyContent(tree, cells);
}

TEST(HashQuadtreeTest, RangesCompliance) {
    static_assert(std::ranges::range<HashQuadtree>);
    static_assert(std::ranges::input_range<HashQuadtree>);

    LifeHashSet cells = { {1, 1}, {2, 2}, {3, 3} };
    HashQuadtree tree{cells};

    // Test with std::algorithms
    auto count = std::count_if(tree.begin(), tree.end(), [](const Vec2& v) {
        return v.X > 1; // Should find {2,2} and {3,3}
    });
    EXPECT_EQ(count, 2);
}