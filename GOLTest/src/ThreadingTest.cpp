#include <gtest/gtest.h>

#include <latch>

#include "SimulationWorker.h"
#include "HashQuadtree.h"

namespace gol
{
  //  TEST(ThreadingTest, CopyTest)
  //  {
  //      std::latch startCounter{ 2 };
  //      
  //      SimulationWorker worker{};
  //      std::jthread workerThread{ [&worker, &startCounter](std::stop_token stopToken)
  //      {
  //          startCounter.count_down();

  //          GameGrid grid{};
  //          
  //          worker.Start(grid, false);
		//} };

		//std::latch latch{ 2 };
  //  }
}