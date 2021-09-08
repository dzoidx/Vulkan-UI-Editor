#pragma once
#include "Types.h"

class GrowingBuffer
{
public:
	GrowingBuffer() = delete;
	GrowingBuffer(uint32 size) : data_(), size_() { EnsureSize(size, false); }
	byte* GetData() { return data_; }
	uint32 GetSize() { return size_; }
	void EnsureSize(uint32 size, bool copy)
	{
		if (size_ >= size)
			return;
		byte* data = (byte*)MemoryManager::Instance()->AllocateRaw(size);
		if (copy && size_)
			memcpy(data, data_, size_);
		if (data_)
		{
			MemoryManager::Instance()->FreeRaw(data_, size_);
		}
		data_ = data;
		size_ = size;
	}
private:
	byte* data_;
	uint32 size_;
};
