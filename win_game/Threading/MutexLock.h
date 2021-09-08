#pragma once
#include "Mutex.h"

class MutexLock
{
public:
	MutexLock(Mutex* mutex);
	void operator=(const MutexLock& m) = delete;
	MutexLock();
	~MutexLock();
	void Lock(Mutex* mutex);
	void Unlock();
private:
	Mutex* mutex_;
};
