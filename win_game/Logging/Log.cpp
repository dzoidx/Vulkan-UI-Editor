#include "Logging.h"
#include "Threading/Thread.h"
#include "Types/DateTime.h"
#include "Utils/StringBuilder.h"

Log logger;

Log::Log()
{
	LoggerName = "Core";
#ifdef _WIN32
	dest_ = (FileStream&&)FileStream::OpenWrite(L"log.txt");
#else
	assert(false);
	//TODO: win пишем в файл под ноги, android пишем в системный лог, ios хз
#endif
}

Log::~Log()
{
}

void Log::Error(const char* msg)
{
	Log_("ERROR", msg);
}

void Log::Warn(const char* msg)
{
	Log_("WARN", msg);
}
void Log::Info(const char* msg)
{
	Log_("INFO", msg);
}
void Log::Debug(const char* msg)
{
	Log_("DEBUG", msg);
}

void Log::Log_(const char* level, const char* msg)
{
	StringBuilder sb("{0} [{1}] {2} [{3}] {4}\n");
	sb.Set(0, DateTime::UtcNow().ToString());
	sb.Set(1, Thread::CurrentThreadId());
	sb.Set(2, level);
	sb.Set(3, LoggerName);
	sb.Set(4, msg);
	dest_.Write((const byte*)sb.ToUtf8(), sb.GetCurrentLen());
}

void Error(const char* msg)
{
	logger.Error(msg);
}
void Warn(const char* msg)
{
	logger.Warn(msg);
}
void Info(const char* msg)
{
	logger.Info(msg);
}
void Debug(const char* msg)
{
	logger.Debug(msg);
}