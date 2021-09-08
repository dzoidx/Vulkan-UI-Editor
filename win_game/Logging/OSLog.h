#pragma once
#include "Types.h"

#ifdef _WIN32
void LogWinError(const char* func, uint32 line);
#define WinError(func) LogWinError(func, __LINE__)
#else
void LogPosixError(const char* func);
void LogPosixError(const char* func, int32 code);
#endif // _WIN32