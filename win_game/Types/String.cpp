#include "String.h"

#include <cassert>
#include <cstring>
#include "List.h"
#include "Utils/StringBuilder.h"
#include "Memory/MemoryManager.h"
#include "Memory/TempMemory.h"


uint32 String::Utf8CharLen(const char* data, int32 offset, int32 len)
{
	uint32 charlen = 0;
	data += offset;
	while (len > 0)
	{
		int32 charBytes;
		if ((*data & 0x80) == 0) charBytes = 1;
		else if ((*data & 0xE0) == 0xC0) charBytes = 2;
		else if ((*data & 0xF0) == 0xE0) charBytes = 3;
		else if ((*data & 0xF8) == 0xF0) charBytes = 4;
		else
			return 0; // unknown format
		len -= charBytes;
		data += charBytes;
		++charlen;
	}
	return charlen;
}
uint32 String::Utf8ToUtf32(const char* data, int32 offset, int32 len, char32* out, uint32 outSize)
{
	uint32 charSize = String::Utf8CharLen(data, offset, len);
	if (charSize > outSize)
		return charSize; // not enough space in output buffer
	data += offset;
	outSize = 0;
	while (len > 0)
	{
		int32 charBytes;
		if ((*data & 0x80) == 0)
		{
			charBytes = 1;
			*out++ = *data;
		}
		else if ((*data & 0xE0) == 0xC0)
		{
			charBytes = 2;
			*out++ = ((*data & 0x1F) << 6) | ((*(data + 1) & 0x3F));
		}
		else if ((*data & 0xF0) == 0xE0)
		{
			charBytes = 3;
			*out++ = ((*data & 0x0F) << 12) | ((*(data + 1) & 0x3F) << 6) | ((*(data + 2) & 0x3F));
		}
		else if ((*data & 0xF8) == 0xF0)
		{
			charBytes = 4;
			*out++ = ((*data & 0x0F) << 18) | ((*(data + 1) & 0x3F) << 12) | ((*(data + 2) & 0x3F) << 6) | ((*(data + 3) & 0x3F));
		}
		else
			return 0; // unknown format
		len -= charBytes;
		data += charBytes;
		++outSize;
	}
	return outSize;
}
uint32 String::Utf32ToUtf8(const char32* data, uint32 len, char* out, uint32 outSize)
{
	int32 bytesSize = 1;
	while (len > 0)
	{
		if (*data < 0x80)
		{
			++bytesSize;
			if (outSize != 0)
			{
				*out++ = (char)*data;
				--outSize;
			}
		}
		else if (*data < 0x0800)
		{
			bytesSize += 2;
			if (outSize > 1)
			{
				*out++ = 192 | (*data >> 6);
				*out++ = 128 | (*data & 63);
				outSize -= 2;
			}
		}
		else if (*data < 0x010000)
		{
			bytesSize += 3;
			if (outSize > 2)
			{
				*out++ = 224 | (*data >> 12);
				*out++ = 128 | ((*data >> 6) & 63);
				*out++ = 128 | (*data & 63);
				outSize -= 3;
			}
		}
		else if (*data < 0x110000)
		{
			bytesSize += 4;
			if (outSize > 3)
			{
				*out++ = 240 | (*data >> 18);
				*out++ = 128 | ((*data >> 12) & 63);
				*out++ = 128 | ((*data >> 6) & 63);
				*out++ = 128 | (*data & 63);
				outSize -= 4;
			}
		}
		else
		{
			assert(false);
		}
		++data;
		--len;
	}
	if (outSize != 0)
		*out = 0;
	return bytesSize;
}
void String::ReverseChars(char32* buff, uint32 size)
{
	int32 hsize = size / 2;
	int32 t;
	for (int32 i = 0; i < hsize; ++i)
	{
		t = buff[i];
		buff[i] = buff[size - i - 1];
		buff[size - i - 1] = t;
	}
}
void String::ReverseChars(char* buff, uint32 size)
{
	int32 hsize = size / 2;
	int32 t;
	for (int32 i = 0; i < hsize; ++i)
	{
		t = buff[i];
		buff[i] = buff[size - i - 1];
		buff[size - i - 1] = t;
	}
}
uint32 String::Len(const char* str)
{
	uint32 len = 0;
	while (*str)
	{
		++str;
		++len;
	}
	return len;
}
uint32 String::Len(const wchar_t* str)
{
	uint32 len = 0;
	while (*str)
	{
		++str;
		++len;
	}
	return len;
}
int32 String::CheckCodesInRange(String& str, uint32 offset, char32 from, char32 to)
{
	assert(offset < str.len_);
	for (uint32 i = offset; i < str.len_; ++i)
	{
		if (str.chars_[i] < from || str.chars_[i] > to)
			return i;
	}
	return -1;
}
int32 String::ReadBytesFromHex(String& str, uint32 offset, List<byte>& list)
{
	byte hc = 0;
	byte c = 0;
	for (uint32 i = offset; i < str.len_; ++i)
	{
		if (str[i] >= '0' && str[i] <= '9')
		{
			hc = str[i] - 0x30;
		}
		else if (str[i] >= 'a' && str[i] <= 'f')
		{
			hc = str[i] - 'a' + 10;
		}
		else if (str[i] >= 'A' && str[i] <= 'F')
		{
			hc = str[i] - 'A' + 10;
		}
		else
			return i;
		if ((i - offset) % 2 == 0)
			c = hc << 4;
		else
		{
			c |= hc;
			list.Add(c);
		}
	}
	return -1;
}

StringBuilder String::Format(const char* format)
{
	return StringBuilder(format);
}

String::String(const char* str)
	:String(str, Len(str))
{

}

String::String(const char* str, int32 len)
{
	int32 buffSz = Utf8CharLen(str, 0, len);
	char32* utf32Buff = MemoryManager::Instance()->Allocate<char32>(buffSz);
	Utf8ToUtf32(str, 0, len, utf32Buff, buffSz);
	uint32 byteSize = 4 * (buffSz + 1);
	String* result = MemoryManager::Instance()->Allocate<String>();
	chars_ = utf32Buff;
	len_ = buffSz;
}

String::String()
{
	chars_ = nullptr;
	len_ = 0;
}

String::String(const String& str)
{
	if (str.chars_ == nullptr)
	{
		chars_ = nullptr;
		len_ = 0;
	}
	else
	{
		chars_ = MemoryManager::Instance()->Allocate<char32>(str.len_);
		memcpy(chars_, str.chars_, sizeof(char32) * str.len_);
		len_ = str.len_;
	}
}

String::~String()
{
	Free();
}

void String::Free()
{
	if (chars_ == nullptr) return;
	MemoryManager::Instance()->Free<char32>(chars_, len_);
	chars_ = nullptr;
	len_ = 0;
}

String& String::operator=(String&& from)
{
	len_ = from.len_;
	chars_ = from.chars_;
	from.len_ = 0;
	from.chars_ = nullptr;
	return *this;
}

String& String::operator=(const String& from)
{
	MemoryManager::Instance()->Free<char32>(chars_, len_);
	chars_ = nullptr;
	len_ = 0;
	if (from.len_ < 1)
		return *this;

	chars_ = MemoryManager::Instance()->Allocate<char32>(from.len_);
	len_ = from.len_;
	memcpy(chars_, from.chars_, len_ * sizeof(char32));

	return *this;
}

char32& String::operator[](uint32 idx)
{
	assert(idx < len_);
	return chars_[idx];
}

String::String(const char32* chars, uint32 count)
{
	chars_ = MemoryManager::Instance()->Allocate<char32>(count);
	len_ = count;
	memcpy(chars_, chars, count * 4);
}

String::String(char32* chars, uint32 count)
{
	chars_ = chars;
	len_ = count;
}

String String::FromInt(int64 num)
{
	char32* buff = new char32[20 * 4];
	int32 pos = 0;
	// -9223372036854775808
	int64 n = num < 0 ? -num : num;
	while (n)
	{
		buff[pos++] = (n % 10) | 0x30;
		n /= 10;
	}
	// num == 0
	if (pos == 0)
	{
		buff[0] = 0x30;
		return String(buff, 1);
	}
	if (num < 0)
		buff[pos++] = L'-';

	ReverseChars(buff, pos);

	return String(buff, pos);
}

String::String(const wchar_t* str)
{
	int len = Len(str);
	char32* buff = MemoryManager::Instance()->Allocate<char32>(len);
	int32 size = 0;
	char32* pos = buff;
	while (*str)
	{
		if (sizeof(wchar_t) >= 4)
		{
			*pos = *str;
		}
		else
		{
			if (*str < 0xD800 || *str > 0xDFFF)
			{
				*pos = *str;
			}
			else
			{
				int code = (*str & 0x03FF) << 10;
				++str;
				if (*str < 0xDC00 || *str > 0xDFFF)
					return; // TODO: error?
				else
				{
					code |= (*str & 0x03FF);
				}
				*pos = code + 0x10000;
			}
		}
		++str;
		++pos;
		++size;
	}
	chars_ = buff;
	len_ = size;
}

const wchar_t* String::ToWchar() const
{
	wchar_t* buff;
	if (sizeof(wchar_t) == sizeof(int))
	{
		buff = GetCurrentGameSate()->tempMemory->Allocate<wchar_t>(len_ + 1);
		memcpy(buff, chars_, sizeof(wchar_t) * len_);
		buff[len_ + 1] = 0;
		return buff;
	}

	uint32 charsLen = 0;
	for (uint32 i = 0; i < len_; ++i)
	{
		if (chars_[i] > 0xFFFF)
			charsLen += 2;
		else
			++charsLen;
	}
	buff = GetCurrentGameSate()->tempMemory->Allocate<wchar_t>(charsLen + 1);
	wchar_t* result = buff;
	buff[charsLen] = 0;

	if (sizeof(wchar_t) == 2)
	{
		for (uint32 i = 0; i < len_; ++i)
		{
			if (chars_[i] < 0x10000)
				*buff++ = (wchar_t)chars_[i];
			else
			{
				int code = chars_[i] - 0x10000;
				*buff++ = 0xD800 | (code >> 10);
				*buff++ = 0xDC00 | (code & 0x3FF);
			}
		}
	}
	return result;
}

const char* String::ToUtf8() const
{
	uint32 size = String::Utf32ToUtf8(chars_, len_, nullptr, 0);
	char* data = currentGameState->tempMemory->Allocate<char>(size);
	String::Utf32ToUtf8(chars_, len_, data, size);
	return data;
}

String String::Reverse()
{
	String s;
	s.chars_ = MemoryManager::Instance()->Allocate<char32>(len_);
	s.len_ = len_;
	ReverseChars(s.chars_, s.len_);
	return s;
}

int32 String::LastIndexOf(const String& n, uint32 offset) const
{
	if (len_ == 0 || n.GetLength() == 0 || offset >= len_ || n.GetLength() > len_)
		return -1;
	uint32 pos = len_ - n.GetLength();
	while (pos > offset)
	{
		if (chars_[pos] == n.chars_[0])
		{
			if (n.len_ == 1)
				return pos;
			if (len_ - pos >= n.GetLength())
			{
				for (uint32 i = 1; i < n.GetLength(); ++i)
				{
					if (chars_[pos + i] != n.chars_[i])
						break;
					if (i == n.GetLength() - 1)
					{
						return pos;
					}
				}
			}
		}
		--pos;
	}
	return -1;
}

int32 String::IndexOf(const String& n, uint32 offset) const
{
	if (len_ == 0 || n.GetLength() == 0 || offset >= len_)
		return -1;
	uint32 pos = offset;
	while (pos < len_)
	{
		if (chars_[pos] == n.chars_[0])
		{
			if (n.len_ == 1)
				return pos;
			if (len_ - pos >= n.GetLength())
			{
				for (uint32 i = 1; i < n.GetLength(); ++i)
				{
					if (chars_[pos + i] != n.chars_[i])
						break;
					if (i == n.GetLength() - 1)
					{
						return pos;
					}
				}
			}
		}
		++pos;
	}
	return -1;
}

String String::Substring(uint32 offset, uint32 len) const
{
	if (offset >= len_)
		return String();
	uint32 tailLen = len_ - offset;
	if (tailLen < len)
		len = tailLen;
	char32* sub = MemoryManager::Instance()->Allocate<char32>(len);
	memcpy(sub, chars_ + offset, len * sizeof(char32));
	return String(sub, len);
}

List<String> String::Split(const String& separator) const
{
	List<String> result;
	uint32 offset = 0;
	int32 findIdx = IndexOf(separator, offset);
	while (findIdx > -1)
	{
		result.Add(Substring(offset, (uint32)findIdx - offset));
		offset = (uint32)findIdx + separator.GetLength();
		findIdx = IndexOf(separator, offset);
	}
	if (offset < len_)
	{
		result.Add(Substring(offset));
	}
	return result;
}

List<String> String::Split(char32 separator) const
{
	String str((const char32*)&separator, 1);
	return Split(str);
}

String& String::ToLower()
{
	for (uint32 i = 0; i < len_; ++i)
	{
		if (chars_[i] > 'A' && chars_[i] < 'Z')
		{
			chars_[i] += 0x20;
		}
	}
	return *this;
}

String& String::operator+=(char32 r)
{
	char32* buff = MemoryManager::Instance()->Allocate<char32>(len_ + 1);
	memcpy(buff, chars_, len_ * sizeof(char32));
	buff[len_] = r;
	++len_;
	MemoryManager::Instance()->Free<char32>(chars_, len_);
	chars_ = buff;

	return *this;
}

String operator+(const String& l, char32 r)
{
	String str(&r, 1);
	return l + str;
}

String operator+(const String& l, const String& r)
{
	uint32 lLen = l.GetLength();
	uint32 rLen = r.GetLength();
	uint32 len = lLen + rLen;
	char32* buff = MemoryManager::Instance()->Allocate<char32>(len * sizeof(int32));

	memcpy(buff, l.chars_, l.len_ * sizeof(char32));
	memcpy(buff + l.len_, r.chars_, r.len_ * sizeof(char32));

	return String(buff, len);
}

int32 String::Compare(const String& l, const String& r)
{
	uint32 commLen = l.len_;
	if (r.len_ < commLen)
		commLen = r.len_;
	for (uint32 i = 0; i < commLen; ++i)
	{
		if (l.chars_[i] == r.chars_[i])
			continue;
		return l.chars_[i] > r.chars_[i] ? 1 : -1;
	}
	if (l.len_ == r.len_)
		return 0;
	return l.len_ > commLen ? 1 : -1;
}

String String::Join(const String& glue, const List<String>& strings)
{
	if (strings.GetLen() == 0)
		return String();
	uint32 totalSize = glue.len_ * (strings.GetLen() - 1);
	for (uint32 i = 0; i < strings.GetLen(); ++i)
	{
		totalSize += strings[i].len_;
	}
	char32* result = MemoryManager::Instance()->Allocate<char32>(totalSize);
	memcpy(result, strings[0].chars_, strings[0].len_ * sizeof(char32));
	uint32 offset = strings[0].len_;
	for (uint32 i = 1; i < strings.GetLen(); ++i)
	{
		memcpy(result + offset, glue.chars_, glue.len_ * sizeof(char32));
		offset += glue.len_;
		memcpy(result + offset, strings[i].chars_, strings[i].len_ * sizeof(char32));
		offset += strings[i].len_;
	}

	return String(result, totalSize);
}