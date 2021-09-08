#include <cassert>
#include "Windows.h"
#include "Timer.h"
#include "Types/DateTime.h"

//TODO: POSIX ticks

int64 SystemTimesFreq;
int64 SystemTimeToMsecs;
int64 SystemTimeToTicks;

#define GET_TICKS_IMPL(name) int64 name(void)
typedef GET_TICKS_IMPL(get_ticks_impl);

GET_TICKS_IMPL(GetTicksLowRes)
{
	LARGE_INTEGER ticks{};
	if (QueryPerformanceCounter(&ticks))
	{
		return ticks.QuadPart * SystemTimeToTicks;
	}
	return 0;
}

GET_TICKS_IMPL(GetTicksHighRes)
{
	LARGE_INTEGER ticks{};
	if (QueryPerformanceCounter(&ticks))
	{
		return ticks.QuadPart / SystemTimeToTicks;
	}
	return 0;
}

GET_TICKS_IMPL(GetTicksExactRes)
{
	LARGE_INTEGER ticks{};
	if (QueryPerformanceCounter(&ticks))
	{
		return ticks.QuadPart;
	}
	return 0;
}

get_ticks_impl* GetTicksImpl;

void WinTimeInit()
{
	LARGE_INTEGER perfFreq;
	if (QueryPerformanceFrequency(&perfFreq))
	{
		SystemTimesFreq = perfFreq.QuadPart;
		if (SystemTimesFreq < TicksPerSecond)
		{
			SystemTimeToTicks = TicksPerSecond / SystemTimesFreq;
			GetTicksImpl = &GetTicksLowRes;
		}
		else if (SystemTimesFreq == TicksPerSecond)
		{
			SystemTimeToTicks = 1;
			GetTicksImpl = &GetTicksExactRes;
		}
		else
		{
			SystemTimeToTicks = SystemTimesFreq / TicksPerSecond;
			GetTicksImpl = &GetTicksHighRes;
		}
		SystemTimeToMsecs = SystemTimesFreq / 1000;
	}
}

int64 GetCurrentMsecs()
{
	LARGE_INTEGER ticks{};
	if (QueryPerformanceCounter(&ticks))
	{
		return ticks.QuadPart / SystemTimeToMsecs;
	}
	return 0;
}

int64 GetCurrentTick()
{
	assert(GetTicksImpl != nullptr);
	return GetTicksImpl();
}