#pragma once
#include "Types.h"

//TODO: ����������, ����������� � DateTime
class TimeSpan
{
public:
	TimeSpan(uint64 time)
		:rawData_(time)
	{}
private:
	uint64 rawData_;
};
