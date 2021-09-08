#include "Mutex.h"
#include "cassert"
#include <pthread.h>

#include "Types.h"


Mutex::Mutex()
{
	int32 r = pthread_mutex_init(&handle_, NULL);
	if (r == 0)
		return;
	LogPosixError("pthread_mutex_init", r);
}

Mutex::~Mutex()
{
	int32 r = pthread_mutex_destroy(&handle_);
	if (r == 0)
		return;
	LogPosixError("pthread_mutex_destroy", r);
}

MutexLock Mutex::Lock()
{
	int32 r = pthread_mutex_lock(&handle_);
	if (r == 0)
		return;
	LogPosixError("pthread_mutex_lock", r);
}

void Mutex::Unlock()
{
	int32 r = pthread_mutex_unlock(&handle_);
	if (r == 0)
		return;
	LogPosixError("pthread_mutex_unlock", r);
}