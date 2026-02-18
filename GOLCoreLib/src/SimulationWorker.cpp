#include "SimulationWorker.h"

#include <chrono>

namespace gol
{
	SimulationWorker::~SimulationWorker()
	{
		m_Running = false;
	}

	void SimulationWorker::Start(GameGrid& initialGrid)
	{
		m_Running = true;

		initialGrid.PrepareCopy();
		auto bufferA = std::make_shared<GameGrid>(initialGrid);
		auto bufferB = std::make_shared<GameGrid>(initialGrid);
		auto bufferC = std::make_shared<GameGrid>(initialGrid);

		m_Snapshot.store(bufferA, std::memory_order_release);
		std::shared_ptr<GameGrid> backBuffer{ bufferB };
		std::shared_ptr<GameGrid> workerGrid{ bufferC };

		m_Thread = std::jthread{ [this, workerGrid, backBuffer]() mutable
		{
			auto nextFrame = std::chrono::steady_clock::now();
			while (m_Running.load(std::memory_order_relaxed))
			{
				workerGrid->Update(m_StepCount.load(std::memory_order_relaxed));
				std::swap(workerGrid, backBuffer);

				workerGrid = m_Snapshot.exchange(backBuffer, std::memory_order_acq_rel);

				nextFrame += std::chrono::milliseconds{ m_TickDelayMs.load(std::memory_order_relaxed) };
				std::this_thread::sleep_until(nextFrame);
			}
			m_Snapshot.load(std::memory_order_acquire)->PrepareCopy();
		} };
	}

	GameGrid SimulationWorker::Stop()
	{
		m_Running = false;
		if (m_Thread.joinable())
			m_Thread.join();

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