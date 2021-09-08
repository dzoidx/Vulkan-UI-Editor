#pragma once
#include "Types.h"

//TODO: реализаци€, интерга€ци€ с DateTime
class TimeSpan
{
public:
	TimeSpan(uint64 time)
		:rawData_(time)
	{}
private:
	uint64 rawData_;
};
