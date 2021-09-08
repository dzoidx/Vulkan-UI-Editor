#pragma once
#include "Types.h"

enum class TouchState
{
	New,
	Continue,
	End
};

struct Touch
{
	int32 X, Y;
	TouchState State;
};
