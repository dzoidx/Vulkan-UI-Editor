#include <cerrno>
#include <string.h>

#include "Logging.h"
#include "OSLog.h"
#include "Utils/StringBuilder.h"

void LogPosixError(const char* func, int32 code)
{
	Error(String::Format("System Error at {0}. {1}")
		.Set(0, func)
		.Set(1, strerror(code))
		.ToUtf8());
}

void LogPosixError(const char* func)
{
	LogPosixError(func, errno);
}

