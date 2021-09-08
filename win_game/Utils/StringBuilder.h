#pragma once
#include "Types.h"
#include "Types/List.h"
#include "Types/String.h"

class StringBuilder
{
public:
	StringBuilder(const char* format);
	StringBuilder& Set(uint32 argN, const char* str);
	StringBuilder& Set(uint32 argN, int64 n);
	StringBuilder& Set(uint32 argN, const String& n);
	const char* ToUtf8();
	uint32 GetCurrentLen() const { return currentLen_; }
private:
	struct Item
	{
		uint32 ArgN;
		const char* Data;
		uint32 Len;
	};
	static uint32 ParseFormat(const char* format, List<Item>& items);
	List<Item> items_;
	uint32 currentLen_;

	static const uint32 NO_ARG = 0xffffffff;
};
