#include "MutexLock.h"

MutexLock::MutexLock(Mutex* mutex) : mutex_(mutex) { mutex_->Lock(); }

MutexLock::MutexLock() : mutex_(nullptr) {}

MutexLock::~MutexLock()
{
	Unlock();
}

void MutexLock::Lock(Mutex* mutex)
{
	if (mutex_)
		return;
	mutex_ = mutex;
	mutex_->Lock();
}

void MutexLock::Unlock()
{
	if (mutex_) {
		mutex_->Unlock();
		mutex_ = nullptr;
	}
}