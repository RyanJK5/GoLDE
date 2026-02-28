#include <gtest/gtest.h>

#include <future>
#include <thread>
#include <latch>

#include "SimulationWorker.h"
#include "RLEEncoder.h"
#include "HashQuadtree.h"

namespace gol
{
    static void StressTest(const std::filesystem::path& universe, int32_t threadCount, int32_t genCount)
    {
        auto decodeResult = RLEEncoder::ReadRegion(universe);
		ASSERT_TRUE(decodeResult.has_value()) << decodeResult.error();

        std::latch startCounter{ threadCount };
        std::latch endCounter{ threadCount };
        std::vector<std::future<GameGrid>> result{};
        for (auto i = 1; i <= threadCount; i++)
        {
            result.emplace_back(std::async(std::launch::async,
                [&startCounter, &endCounter, genCount, threadCount, i, myData = decodeResult->Grid] mutable
                {
                    startCounter.count_down();
                    startCounter.wait();

                    for (auto j = 0; j < (genCount % i); j++)
                        myData.Update(1);
                    for (auto j = 0; j < (genCount / i); j++)
                        myData.Update(i);

                    myData.PrepareCopyAcrossThread();
                    endCounter.count_down();
                    return myData;
                }));
        }

		decodeResult->Grid.Update(genCount);
        endCounter.wait();

        bool allMatch = std::ranges::all_of(result, [&](auto& future)
        {
            return future.get().Data() == decodeResult->Grid.Data();
        });

        EXPECT_TRUE(allMatch);
    }

    TEST(ThreadingTest, CopyTest)
    {
        const auto decodeResult = RLEEncoder::ReadRegion(
            std::filesystem::path{ "universes" } / "squiggles1.gol");
        ASSERT_TRUE(decodeResult.has_value()) << decodeResult.error();
        
        std::latch counter{ 2 };
        auto result1 = std::async(std::launch::async, [&counter, myData = decodeResult->Grid] mutable
        {
            counter.count_down();
            counter.wait();

            myData.Update(200);
            myData.PrepareCopyAcrossThread();
            return myData;
        });
        auto result2 = std::async(std::launch::async, [&counter, myData = decodeResult->Grid] mutable
        {
            counter.count_down();
            counter.wait();

            myData.Update(100);
            myData.Update(100);
            myData.PrepareCopyAcrossThread();
            return myData;
        });

        EXPECT_EQ(result1.get().Data(), result2.get().Data());
    }

    TEST(ThreadingTest, Simulate10Squiggles)
    {
		StressTest(std::filesystem::path{ "universes" } / "squiggles1.gol", 10, 20);
    }

    TEST(ThreadingTest, Simulate16BigSquiggles)
    {
        StressTest(std::filesystem::path{ "universes" } / "bigsquiggles1.gol", 16, 16);
    }

    TEST(ThreadingTest, SimulateBreeders)
    {
        StressTest(std::filesystem::path{ "universes" } / "glider_gun.gol", 20, 4096);
    }
}