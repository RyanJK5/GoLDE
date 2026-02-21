#include "SimulationWorker.h"

#include <condition_variable>
#include <chrono>

namespace gol
{
	void SimulationWorker::Start(GameGrid& initialGrid, bool oneStep, const std::function<void()>& onStop)
	{
		initialGrid.PrepareCopy();
		auto bufferA = std::make_shared<GameGrid>(initialGrid);
		auto bufferB = std::make_shared<GameGrid>(initialGrid);
		auto bufferC = std::make_shared<GameGrid>(initialGrid);

		m_Snapshot.store(bufferA, std::memory_order_release);
		std::shared_ptr<GameGrid> backBuffer{ bufferB };
		std::shared_ptr<GameGrid> workerGrid{ bufferC };

		m_Thread = std::jthread{ 
		[this, workerGrid, backBuffer, oneStep, onStop](std::stop_token stopToken) mutable
		{
			std::condition_variable_any sleepCondition{};
			std::mutex sleepMutex{};

			auto nextFrame = std::chrono::steady_clock::now();
			while(true)
			{
				m_LastUpdate.store(std::chrono::steady_clock::now(), std::memory_order_release);

				workerGrid->Update(m_StepCount.load(std::memory_order_relaxed), stopToken);

				if (stopToken.stop_requested())
					break;

				std::swap(workerGrid, backBuffer);
				workerGrid = m_Snapshot.exchange(backBuffer, std::memory_order_acq_rel);

				if (oneStep)
					break;

				const auto tickDelayMs = m_TickDelayMs.load(std::memory_order_relaxed);
				if (tickDelayMs > 0)
				{
					nextFrame += std::chrono::milliseconds{ tickDelayMs };
					
					std::unique_lock lock{ sleepMutex };
					bool stopRequested = sleepCondition.wait_until(lock, stopToken, nextFrame, [] { return false; });

					if (stopRequested)
						break;
				}
			}

			m_Snapshot.load(std::memory_order_acquire)->PrepareCopy();

			if (oneStep && !stopToken.stop_requested())
				onStop();
		} };
	}

	GameGrid SimulationWorker::Stop()
	{
		if (m_Thread.joinable())
		{
			m_Thread.request_stop();
			m_Thread.join();
		}

		auto shared = m_Snapshot.load(std::memory_order_relaxed);
		auto ret = std::move(*shared);
		m_Snapshot.store(nullptr, std::memory_order_release);
		return ret;
	}

	void SimulationWorker::SetStepCount(int64_t stepCount)
	{
		m_StepCount.store(stepCount, std::memory_order_relaxed);
	}

	void SimulationWorker::SetTickDelayMs(int64_t tickDelayMs)
	{
		m_TickDelayMs.store(tickDelayMs, std::memory_order_relaxed);
	}

	std::shared_ptr<GameGrid> SimulationWorker::GetResult() const
	{
		return m_Snapshot.load(std::memory_order_acquire);
	}

	std::chrono::duration<float> SimulationWorker::GetTimeSinceLastUpdate() const
	{
		return std::chrono::steady_clock::now() - m_LastUpdate.load(std::memory_order_acquire);
	}
}