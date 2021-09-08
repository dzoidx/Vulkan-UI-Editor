#pragma once
#include <atomic>
#include "Types.h"
#include "Threading/Mutex.h"
#include "Types/Buffer.h"

struct MemoryBlock
{
	void* Data;
	uint32 Size;
};

void* MemoryAllocate(uint64 bytesCount, bool temp = true);
int32 MemoryFree(void* memory);

void* operator new(size_t size);
void operator delete(void* p) noexcept;

class MemoryManager
{
public:
	static MemoryManager* Instance();

	// аллокация без вызова конструкторов
	void* AllocateRaw(uint32 size);
	// деаллокация без вызова деструкторов
	void FreeRaw(void* mem, uint32 size);

	Buffer AllocateBuffer(int32 size);
	void FreeMemory(MemoryDesc* mem);
	void* Allocate(int32 size);
	void Free(void* memory);
	template<typename T>
	T* Allocate()
	{
		T* result = new T();
		allocatedBytes_ += sizeof(T);
		return result;
	}

	template<typename T>
	T* Allocate(uint32 count)
	{
		T* result = new T[count];
		allocatedBytes_ += sizeof(T) * count;
		return result;
	}

	template<typename T>
	void Free(T* mem)
	{
		if (mem == nullptr) return;
		delete mem;
		freedBytes_ += sizeof(T);
	}

	template<typename T>
	void Free(T* mem, uint32 count)
	{
		if (mem == nullptr) return;
		delete[] mem;
		freedBytes_ += sizeof(T) * count;
	}

	int64 GetAllocatedBytes() const { return allocatedBytes_; }
	int64 GetFreedBytes() const { return freedBytes_; }

private:
	std::atomic<int64> allocatedBytes_;
	std::atomic<int64> freedBytes_;

	bool cacheEnabled_ = false;
	Mutex cacheLock_;
	uint32 cacheSize_;
	static const uint32 cacheMaxSize_ = 1024;
	MemoryDesc* cache_[cacheMaxSize_] = {};

	static MemoryManager* instance;
};


template<typename T>
struct FMemory
{
	T* Data;
	uint32 Count;
};

template<typename T>
inline FMemory<T> FMemory_Alloc(uint32 count)
{
	return FMemory<T> {(T*)malloc(count * sizeof(T)), count};
}

template<typename T>
inline void FMemory_Resize(FMemory<T>* mem, uint32 count)
{
	assert(count > mem->Count);
	T* buff = malloc(count * sizeof(T));
	memcpy(buff, mem->Data, sizeof(T) * mem->Count);
	free(mem->Data);
	mem->Data = buff;
	mem->Count = count;
}