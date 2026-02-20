#include "SimulationWorker.h"

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

		m_Thread = std::jthread{ [this, workerGrid, backBuffer, oneStep, onStop]
		(std::stop_token stopToken) mutable
		{
			auto nextFrame = std::chrono::steady_clock::now();
			while(true)
			{
				workerGrid->Update(m_StepCount.load(std::memory_order_relaxed), stopToken);

				if (stopToken.stop_requested())
					break;

				std::swap(workerGrid, backBuffer);

				workerGrid = m_Snapshot.exchange(backBuffer, std::memory_order_acq_rel);

				if (oneStep)
					break;

				nextFrame += std::chrono::milliseconds{ m_TickDelayMs.load(std::memory_order_relaxed) };
				std::this_thread::sleep_until(nextFrame);
			}

			m_Snapshot.load(std::memory_order_acquire)->PrepareCopy();

			if (oneStep)
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

		auto shared = m_Snapshot.load(std::memory_order_acquire);
		auto ret = std::move(*shared);
		m_Snapshot.store(nullptr, std::memory_order_release);
		return ret;
	}

	void SimulationWorker::SetStepCount(int64_t stepCount)
	{
		m_StepCount = stepCount;
	}

	void SimulationWorker::SetTickDelayMs(int64_t tickDelayMs)
	{
		m_TickDelayMs = tickDelayMs;
	}

	std::shared_ptr<GameGrid> SimulationWorker::GetResult() const
	{
		return m_Snapshot.load(std::memory_order_acquire);
	}
}