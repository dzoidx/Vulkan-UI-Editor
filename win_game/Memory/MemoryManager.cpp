// (c) 2021 Alexander Morgan (dzoidx@gmail.com). All rights reserved.

#include "MemoryManager.h"
#include "Threading/MutexLock.h"

MemoryManager* MemoryManager::instance = nullptr;

void* MemoryAllocate(uint64 bytesCount, bool temp)
{
	return malloc(bytesCount);
}

inline int32 MemoryFree(void* memory)
{
	free(memory);
	return 0;
}

void* operator new(size_t size)
{
	void* p = MemoryAllocate(size, false);
	return p;
}

void operator delete(void* p) noexcept
{
	MemoryFree(p);
}

MemoryManager* MemoryManager::Instance()
{
	static Mutex instanceLock;
	if (instance != nullptr)
		return instance;
	MutexLock lock;
	lock.Lock(&instanceLock);
	if (instance != nullptr)
		return instance;
	instance = new MemoryManager();
	return instance;
}

Buffer MemoryManager::AllocateBuffer(int32 size)
{
	MemoryDesc* mem;
	MutexLock lockCache(&cacheLock_);
	if (cacheSize_ && cacheEnabled_)
	{
		int32 i = 0;
		for (i; i < cacheMaxSize_; ++i)
		{
			if (cache_[i] != nullptr)
			{
				if (cache_[i]->Size >= size)
				{
					Buffer buffer = Buffer(cache_[i]);
					cache_[i] = nullptr;
					--cacheSize_;
					if (buffer.GetSize() == size)
						return buffer;
					return buffer.GetRegion(size);
				}
			}
		}
	}

	mem = new MemoryDesc();
	mem->Data = new byte[size];
	mem->Size = size;
	allocatedBytes_ += size;
	return Buffer(mem);
}

void MemoryManager::FreeMemory(MemoryDesc* mem)
{
	MutexLock lockCache(&cacheLock_);
	uint32 freeSpace = cacheMaxSize_ - cacheSize_;
	if (freeSpace && cacheEnabled_)
	{
		int32 i = 0;
		for (i; i < cacheMaxSize_; ++i)
		{
			if (cache_[i] == nullptr)
				break;
		}
		if (i < cacheMaxSize_)
		{
			cache_[i] = mem;
			++cacheSize_;
			return;
		}
	}
	MemoryDesc* oldMem = cacheEnabled_ ? cache_[0] : mem;
	MutexLock lock;
	lock.Lock(&oldMem->Lock);
	freedBytes_ += oldMem->Size;
	delete[] oldMem->Data;
	if (cacheEnabled_)
	{
		cache_[0] = cache_[cacheMaxSize_ - 1];
		cache_[cacheMaxSize_ - 1] = mem;
	}
}

void* MemoryManager::AllocateRaw(uint32 size)
{
	allocatedBytes_ += size;
	return malloc(size);
}

void MemoryManager::FreeRaw(void* mem, uint32 size)
{
	freedBytes_ += size;
	free(mem);
}

void* MemoryManager::Allocate(int32 size)
{
	return new byte[size];
}

void MemoryManager::Free(void* memory)
{
	delete[] memory;
}