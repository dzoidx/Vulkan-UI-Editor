#include "Logging.h"
#include "OSLog.h"
#include "Windows.h"
#include "Memory/TempMemory.h"
#include "Utils/StringBuilder.h"

void LogWinError(const char* func, uint32 line)
{
	LPVOID lpMsgBuf = currentGameState->tempMemory->Allocate<char>(Kilobytes(64));
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_ENGLISH_US),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	Error(String::Format("WIN Error at {0} {1}. {2}")
		.Set(0, func)
		.Set(1, line)
		.Set(2, (char*)lpMsgBuf)
		.ToUtf8());
}
