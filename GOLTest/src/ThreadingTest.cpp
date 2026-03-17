#include <gtest/gtest.h>

#include <format>
#include <latch>
#include <thread>

#include "HashQuadtree.hpp"
#include "RLEEncoder.hpp"
#include "SimulationWorker.hpp"

namespace gol {
static void StressTest(const std::filesystem::path& universe,
                       int32_t threadCount, int32_t genCount) {
    auto decodeResult = RLEEncoder::ReadRegion(universe);
    ASSERT_TRUE(decodeResult.has_value()) << decodeResult.error();

    std::latch startCounter{threadCount};
    std::vector<std::shared_ptr<GameGrid>> results(threadCount);
    std::vector<std::jthread> threads{};

    for (auto i = 1; i <= threadCount; i++) {
        auto snapshot = std::make_shared<GameGrid>(decodeResult->Grid);
        results[i - 1] = snapshot;

        threads.emplace_back([&startCounter, genCount, i, snapshot] {
            startCounter.count_down();
            startCounter.wait();

            for (auto j = 0; j < (genCount % i); j++) {
                snapshot->Update(1);
            }
            for (auto j = 0; j < (genCount / i); j++) {
                snapshot->Update(i);
            }

            snapshot->PrepareCopyBetweenThreads();
        });
    }

    threads.clear();

    decodeResult->Grid.Update(genCount);

    const bool allMatch = std::ranges::all_of(results, [&](auto& shared) {
        GameGrid local{*shared};
        return local.Data() == decodeResult->Grid.Data();
    });

    EXPECT_TRUE(allMatch);
}

TEST(ThreadingTest, CopyTest) {
    const auto decodeResult = RLEEncoder::ReadRegion(
        std::filesystem::path{"universes"} / "squiggles1.gol");
    ASSERT_TRUE(decodeResult.has_value()) << decodeResult.error();

    auto grid1 = decodeResult->Grid;
    auto grid2 = decodeResult->Grid;
    std::latch counter{2};
    auto result1 = std::jthread([&counter, &grid1] {
        counter.count_down();
        counter.wait();

        const auto id = std::this_thread::get_id();
        std::cout << std::format("ABC: {}\n", id);

        grid1.Update(200);
        grid1.PrepareCopyBetweenThreads();
    });
    auto result2 = std::jthread([&counter, &grid2] {
        counter.count_down();
        counter.wait();

        const auto id = std::this_thread::get_id();
        std::cout << std::format("DEF: {}\n", id);

        grid2.Update(100);
        grid2.Update(100);
        grid2.PrepareCopyBetweenThreads();
    });
    result1.join();
    result2.join();
    std::cout << "Brother\n";

    EXPECT_EQ(GameGrid{grid1}.Data(), GameGrid{grid2}.Data());
}

TEST(ThreadingTest, Simulate10Squiggles) {
    StressTest(std::filesystem::path{"universes"} / "squiggles1.gol", 10, 20);
}

TEST(ThreadingTest, Simulate16BigSquiggles) {
    StressTest(std::filesystem::path{"universes"} / "bigsquiggles1.gol", 16,
               16);
}

TEST(ThreadingTest, SimulateBreeders) {
    StressTest(std::filesystem::path{"universes"} / "glider_gun.gol", 20, 4096);
}
} // namespace gol
