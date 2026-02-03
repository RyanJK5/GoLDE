#ifndef __SimulationWorker_h__
#define __SimulationWorker_h__

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>

#include "GameGrid.h"

namespace gol
{
	class SimulationWorker
	{
	public:
		~SimulationWorker();

		void Start(const GameGrid& initialGrid);
		GameGrid Stop();

		void SetStepCount(int64_t stepCount);
		void SetTickDelayMs(int64_t tickDelayMs);

		std::shared_ptr<GameGrid> GetResult() const;
	private:
		std::atomic<int64_t> m_StepCount = 1;
		std::atomic<int64_t> m_TickDelayMs = 0;

		std::jthread m_Thread;
		std::atomic<std::shared_ptr<GameGrid>> m_Snapshot;
		std::atomic<bool> m_Running{ false };
	};
}

#endif