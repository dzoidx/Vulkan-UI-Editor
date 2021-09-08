#include "StringBuilder.h"

#include <cstring>

#include "Memory/TempMemory.h"


StringBuilder::StringBuilder(const char* format)
{
	currentLen_ = ParseFormat(format, items_);
}

StringBuilder& StringBuilder::Set(uint32 argN, const char* str)
{
	if (argN == StringBuilder::NO_ARG)
		return *this;
	for (uint32 i = 0; i < items_.GetLen(); ++i)
	{
		if (items_[i].ArgN == argN)
		{
			currentLen_ -= items_[i].Len;
			items_[i].Data = str;
			items_[i].Len = String::Len(items_[i].Data);
			currentLen_ += items_[i].Len;
			return *this;
		}
	}
	return *this;
}

StringBuilder& StringBuilder::Set(uint32 argN, int64 n)
{
	String s = String::FromInt(n);
	return Set(argN, s);
}

StringBuilder& StringBuilder::Set(uint32 argN, const String& n)
{
	if (argN == StringBuilder::NO_ARG)
		return *this;
	for (uint32 i = 0; i < items_.GetLen(); ++i)
	{
		if (items_[i].ArgN == argN)
		{
			currentLen_ -= items_[i].Len;
			items_[i].Data = n.ToUtf8();
			items_[i].Len = String::Len(items_[i].Data);
			currentLen_ += items_[i].Len;
			return *this;
		}
	}
	return *this;
}

const char* StringBuilder::ToUtf8()
{
	char* out = currentGameState->tempMemory->Allocate<char>(currentLen_ + 1);
	char* pos = out;
	for (uint32 i = 0; i < items_.GetLen(); ++i)
	{
		memcpy(pos, items_[i].Data, items_[i].Len);
		pos += items_[i].Len;
	}
	*pos = 0;
	return out;
}

uint32 StringBuilder::ParseFormat(const char* format, List<Item>& items)
{
	enum class State
	{
		String,
		Arg
	};
	uint32 len = String::Len(format);
	uint32 start = 0;
	uint32 end = 0;
	State state = State::String;
	uint32 argN = 0;
	uint32 totalLen = 0;
	while (end < len)
	{
		switch (state)
		{
		case State::String:
			if (format[end] == '{')
			{
				if (end > start)
				{
					uint32 strlen = end - start;
					items.Add(Item{ StringBuilder::NO_ARG, &format[start], strlen });
					totalLen += strlen;
				}
				start = end++;
				state = State::Arg;
			}
			else
			{
				++end;
			}
			break;
		case State::Arg:
			if (format[end] == '}')
			{
				uint32 strlen = ++end - start;
				items.Add(Item{ argN, &format[start], strlen });
				totalLen += strlen;
				state = State::String;
				start = end;
				argN = 0;
			}
			else
			{
				uint32 decimal = format[end++] - '0';
				argN = argN * 10 + decimal;
			}
			break;
		}
	}
	if (start != end)
	{
		uint32 strlen = end - start;
		items.Add(Item{ StringBuilder::NO_ARG, &format[start], strlen });
		totalLen += strlen;
	}
	return totalLen;
}