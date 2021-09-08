#pragma once
#include "Types.h"

class TempMemory;

struct GameState
{
	int64 FrameCount;
	uint32 Seed;
	TempMemory* tempMemory;
};
