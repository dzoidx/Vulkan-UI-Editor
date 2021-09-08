#pragma once
#include "Types.h"

class Thread
{
public:
	static uint32 CurrentThreadId();
	static void Sleep(uint32 milliseconds);
};
