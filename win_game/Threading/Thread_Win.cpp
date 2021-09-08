#include "Thread.h"
#include "Windows.h"

uint32 Thread::CurrentThreadId()
{
	return (uint32)GetCurrentThreadId();
}

void Thread::Sleep(uint32 milliseconds)
{
	::Sleep(milliseconds);
}