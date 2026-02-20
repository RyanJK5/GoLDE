#ifndef __SimulationWorker_h__
#define __SimulationWorker_h__

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>

#include "GameGrid.h"
#include "HashQuadtree.h"

namespace gol
{
	class SimulationWorker
	{
	public:
		void Start(GameGrid& initialGrid, bool oneStep = false, const std::function<void()>& onStop = {});
		GameGrid Stop();

		void SetStepCount(int64_t stepCount);
		void SetTickDelayMs(int64_t tickDelayMs);

		std::shared_ptr<GameGrid> GetResult() const;
		std::chrono::duration<float> GetTimeSinceLastUpdate() const;
	private:
		std::atomic<int64_t> m_StepCount = 1;
		std::atomic<int64_t> m_TickDelayMs = 0;

		std::atomic<std::chrono::steady_clock::time_point> m_LastUpdate;

		std::atomic<std::shared_ptr<GameGrid>> m_Snapshot;
		
		std::jthread m_Thread;
	};
}

#endif