#pragma once
#include "List.h"
#include "Types.h"

class StringBuilder;

class String
{
public:
	String();
	String(const char32* chars, uint32 count);
	String(char32* chars, uint32 count);
	String(const char* str); // utf8
	String(const char* str, int32 len); // utf8
	String(const String& str);
	String(const wchar_t* str);
	// аллоцирует память во временном хранилище
	const wchar_t* ToWchar() const;
	const char* ToUtf8() const;
	~String();

	String Reverse();
	int32 LastIndexOf(const String& n, uint32 offset) const;
	int32 IndexOf(const String& n, uint32 offset) const;
	String Substring(uint32 offset) const { return Substring(offset, len_ - offset); }
	String Substring(uint32 offset, uint32 len) const;
	List<String> Split(const String& separator) const;
	List<String> Split(char32 separator) const;

	// размер строки без \0
	uint32 GetLength() const { return len_; }
	bool IsEmpty() const { return len_ == 0; }
	// только для ASCII
	String& ToLower();
	String ToLower() const { return String(*this).ToLower(); }

	String& operator=(String&& from);
	String& operator=(const String& from);

	char32& operator[](uint32 idx);

	String& operator+=(char32 r);
	friend String operator+(const String& l, char32 r);
	friend String operator+(const String& l, const String& r);
	friend bool operator==(const String& l, const String& r) { return String::Compare(l, r) == 0; }
	friend bool operator!=(const String& l, const String& r) { return String::Compare(l, r) != 0; }
	friend bool operator>(const String& l, const String& r) { return String::Compare(l, r) == 1; }
	friend bool operator>=(const String& l, const String& r) { return String::Compare(l, r) > -1; }
	friend bool operator<(const String& l, const String& r) { return String::Compare(l, r) == -1; }
	friend bool operator<=(const String& l, const String& r) { return String::Compare(l, r) < 1; }

	static String FromInt(int64 num);
	static uint32 Utf8CharLen(const char* data, int32 offset, int32 len);
	static uint32 Utf8ToUtf32(const char* data, int32 offset, int32 len, char32* out, uint32 outSize);
	static uint32 Utf32ToUtf8(String& str, char* out, uint32 outSize) { return Utf32ToUtf8(str.chars_, str.len_, out, outSize); }
	static uint32 Utf32ToUtf8(const char32* data, uint32 len, char* out, uint32 outSize);
	static void ReverseChars(char32* buff, uint32 size);
	static void ReverseChars(char* buff, uint32 size);
	static uint32 Len(const char* str);
	static uint32 Len(const wchar_t* str);
	static int32 CheckCodesInRange(String& str, uint32 offset, char32 from, char32 to);
	static int32 ReadBytesFromHex(String& str, uint32 offset, List<byte>& list);
	static StringBuilder Format(const char* format);
	static int32 Compare(const String& l, const String& r);
	static String Join(const String& glue, const List<String>& strings);
private:
	void Free();
private:
	uint32 len_;
	char32* chars_; // без \0 в конце
};
