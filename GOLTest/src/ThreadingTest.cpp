#include <gtest/gtest.h>

#include <future>
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

            for (auto j = 0; j < (genCount % i); j++)
                snapshot->Update(1);
            for (auto j = 0; j < (genCount / i); j++)
                snapshot->Update(i);

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

    std::latch counter{2};
    auto result1 = std::async(std::launch::async,
                              [&counter, myData = decodeResult->Grid] mutable {
                                  counter.count_down();
                                  counter.wait();

                                  myData.Update(200);
                                  myData.PrepareCopyBetweenThreads();
                                  return myData;
                              });
    auto result2 = std::async(std::launch::async,
                              [&counter, myData = decodeResult->Grid] mutable {
                                  counter.count_down();
                                  counter.wait();

                                  myData.Update(100);
                                  myData.Update(100);
                                  myData.PrepareCopyBetweenThreads();
                                  return myData;
                              });

    EXPECT_EQ(result1.get().Data(), result2.get().Data());
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
