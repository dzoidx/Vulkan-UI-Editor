#pragma once
#ifdef _WIN32
#include "Windows.h"
#endif
#include <cassert>
#include "State.h"
#include "Types.h"

class TempMemory
{
public:
	template<typename T>
	T* Allocate(uint32 count)
	{
		if (currentFrame_ != GetCurrentGameSate()->FrameCount)
		{
			pos_ = 0;
			currentFrame_ = GetCurrentGameSate()->FrameCount;
		}
		uint32 byteSize = count * sizeof(T);
		assert(pos_ + byteSize < totalSize_);
		T* result = (T*)(memory_ + pos_);
		pos_ += byteSize;
		return result;
	}
	TempMemory(uint32 totalSize)
	{
		totalSize_ = totalSize;
		memory_ = (byte*)Allocate(totalSize);
		currentFrame_ = GetCurrentGameSate()->FrameCount;
	}

	static void* Allocate(uint32 size)
	{
#ifdef _WIN32
		void* result = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		assert(result);
		return result;
#endif // _WIN32
	}

	uint32 totalSize_;
	byte* memory_;
	uint32 pos_;
	uint64 currentFrame_;
};
