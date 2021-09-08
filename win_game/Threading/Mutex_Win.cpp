#include "Mutex.h"
#include "cassert"
#include "Logging.h"

Mutex::Mutex()
{
	SECURITY_ATTRIBUTES sa{};
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	handle_ = CreateMutex(&sa, FALSE, NULL);
	if (handle_ == NULL)
	{
		assert(false);
		WinError("CreateMutex");
	}
}

Mutex::~Mutex()
{
	BOOL r = CloseHandle(handle_);
	if (r == FALSE)
	{
		WinError("CloseHandle");
	}
}

void Mutex::Lock()
{
	DWORD r = WaitForSingleObject(handle_, INFINITE);
	if (r == WAIT_FAILED)
	{
		WinError("WaitForSingleObject");
	}
	if (r == WAIT_ABANDONED)
	{
		Warn("Abandoned mutex.");
	}
}

void Mutex::Unlock()
{
	BOOL r = ReleaseMutex(handle_);
	if (r == FALSE)
	{
		WinError("ReleaseMutex");
	}
}

/*
bool Mutex::TryLock(uint32 msecs, MutexLock& lock)
{
	DWORD r = WaitForSingleObject(handle_, (DWORD)msecs);
	if (r == WAIT_FAILED)
	{
		LogWinError("WaitForSingleObject");
		return false;
	}
	if (r == WAIT_ABANDONED)
	{
		Warn("Abandoned mutex.");
	}
	lock = MutexLock(this);
	return r != WAIT_TIMEOUT;
}*/