#include "SimulationWorker.hpp"

#include <chrono>
#include <condition_variable>
#include <print>

namespace gol {
void SimulationWorker::Start(GameGrid& initialGrid, bool oneStep,
                             const std::function<void()>& onStop) {
    initialGrid.PrepareCopyBetweenThreads();
    m_Buffers[0] = initialGrid;
    m_Buffers[1] = initialGrid;
    m_Buffers[2] = initialGrid;

    m_SnapshotIndex.store(0, std::memory_order_release);

    m_Thread = std::jthread{[this, oneStep, onStop](std::stop_token stopToken) {
        auto workerIndex = 1UZ;
        auto backIndex = 2UZ;

        std::condition_variable_any sleepCondition{};
        std::mutex sleepMutex{};
        auto nextFrame = std::chrono::steady_clock::now();

        while (true) {
            m_LastUpdate.store(std::chrono::steady_clock::now(),
                               std::memory_order_relaxed);

            BigInt stepCount = [&] {
                std::scoped_lock locK{m_StepCountMutex};
                return m_StepCount;
            }();
            m_Buffers[workerIndex].Update(stepCount, stopToken);

            if (stopToken.stop_requested()) {
                break;
            }

            // Publish workerIndex as the new snapshot, get back the old one
            backIndex = m_SnapshotIndex.exchange(workerIndex,
                                                 std::memory_order_acq_rel);
            workerIndex = backIndex;

            if (oneStep) {
                break;
            }

            const auto tickDelayMs =
                m_TickDelayMs.load(std::memory_order_relaxed);
            if (tickDelayMs > 0) {
                nextFrame += std::chrono::milliseconds{tickDelayMs};
                std::unique_lock lock{sleepMutex};
                bool stopRequested = sleepCondition.wait_until(
                    lock, stopToken, nextFrame, [] { return false; });
                if (stopRequested) {
                    break;
                }
            }
        }

        m_Buffers[m_SnapshotIndex.load(std::memory_order_acquire)]
            .PrepareCopyBetweenThreads();
        if (oneStep && !stopToken.stop_requested())
            onStop();
    }};
}

GameGrid SimulationWorker::Stop() {
    if (m_Thread.joinable()) {
        m_Thread.request_stop();
        m_Thread.join();
    }
    return m_Buffers[m_SnapshotIndex.load(std::memory_order_relaxed)];
}

void SimulationWorker::SetStepCount(const BigInt& stepCount) {
    std::scoped_lock lock{m_StepCountMutex};
    m_StepCount = stepCount;
}

void SimulationWorker::SetTickDelayMs(int64_t tickDelayMs) {
    m_TickDelayMs.store(tickDelayMs, std::memory_order_relaxed);
}

const GameGrid* SimulationWorker::GetResult() const {
    if (!m_Thread.joinable()) {
        return nullptr;
    }
    return &m_Buffers[m_SnapshotIndex.load(std::memory_order_acquire)];
}

std::chrono::duration<float> SimulationWorker::GetTimeSinceLastUpdate() const {
    return std::chrono::steady_clock::now() -
           m_LastUpdate.load(std::memory_order_relaxed);
}
} // namespace gol
