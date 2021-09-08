#pragma once
#include "IO/FileStream.h"
#include "Logging/OSLog.h"

class Log
{
public:
	Log();
	~Log();
	void Error(const char* msg);
	void Warn(const char* msg);
	void Info(const char* msg);
	void Debug(const char* msg);

private:
	const char* LoggerName;
	void Log_(const char* level, const char* msg);
	FileStream dest_;
};

// пишем в основной лог
void Error(const char* msg);
void Warn(const char* msg);
void Info(const char* msg);
void Debug(const char* msg);

extern Log logger;