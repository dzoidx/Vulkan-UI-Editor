#pragma once
#ifdef _WIN32
#include "Windows.h"
#endif

class Mutex
{
public:
	Mutex();
	~Mutex();
	void Lock();
	//bool TryLock(uint32 msecs, MutexLock& lock); не поддерживается в pthreads
	void Unlock();
private:
#ifdef _WIN32
	HANDLE handle_;
#endif
#ifdef POSIX
	pthread_mutex_t handle_;
#endif
};
