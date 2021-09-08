#pragma once
#include "Types.h"

int64 GetCurrentMsecs();
int64 GetCurrentTick();
inline uint64 GetCurrentNanos() { return GetCurrentTick() * 100; }
#ifdef _WIN32
void WinTimeInit();
#endif // _WIN32
