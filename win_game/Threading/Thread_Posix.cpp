#include <cassert>
#include <time.h>
#include "Thread.h"
#include <unistd.h>

uint32 Thread::CurrentThreadId()
{
	return (uint32)gettid(); // не поддерживается в MacOS
}

void Thread::Sleep(uint32 milliseconds)
{
	timespec spec;
	spec.tv_sec = milliseconds / 1000;
	spec.tv_nsec = (milliseconds - spec.tv_sec * 1000) * 1000000;
	int32 r = nanosleep(&spec, nullptr);
	assert(r == 0);
}