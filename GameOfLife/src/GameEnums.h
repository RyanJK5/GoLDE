#ifndef __GameEnums_h__
#define __GameEnums_h__

namespace gol
{
	enum class GameState
	{
		Paint, Simulation, Paused, Empty
	};

	enum class GameAction
	{
		None, 
		Start, 
		Pause, 
		Resume, 
		Restart, 
		Reset, 
		Clear,

		Step, 
		Resize,
		
		Copy, 
		Cut, 
		Paste
	};

	enum class EditorMode
	{
		None, Insert, Delete, Select
	};
}

#endif