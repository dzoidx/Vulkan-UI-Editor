#pragma once
#include "Types.h"
#include "Threading/Mutex.h"

struct MemoryDesc
{
	void* Data;
	int32 Size;
	int32 UseCount;
	Mutex Lock;
};

class Buffer
{
public:
	Buffer(MemoryDesc* memory, int32 offset, int32 size);
	Buffer(MemoryDesc* memory);
	Buffer(const Buffer& buff);
	~Buffer();

	Buffer& operator=(const Buffer& buff);
	Buffer GetRegion(int size);
	int32 GetSize() const { return size_; }
	void* GetData() const;
	int32 UseCount() const;
	// Все операции над буфером непотокобезопасны.
	// Используй этот вызов если есть основания считать что буфер используется в нескольких потоках.
	void Lock();
	void Unlock();

	static const Buffer Empty;
protected:
	MemoryDesc* mem_;
	int32 offset_;
	int32 size_;
};
