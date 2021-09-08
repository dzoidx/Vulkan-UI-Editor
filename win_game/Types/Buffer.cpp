#include "Buffer.h"
#include <cassert>
#include "Memory/MemoryManager.h"
#include "Threading/MutexLock.h"

const Buffer Buffer::Empty = MemoryManager::Instance()->AllocateBuffer(0);

Buffer::Buffer(MemoryDesc* memory, int32 offset, int32 size)
{
	MutexLock lock(&memory->Lock);
	++memory->UseCount;
	assert(memory->Size - offset >= size);
	mem_ = memory;
	size_ = size;
	offset_ = offset;
}

Buffer::Buffer(MemoryDesc* memory)
	: Buffer(memory, 0, memory->Size)
{
}

Buffer::Buffer(const Buffer& buff)
	: Buffer(buff.mem_)
{
}

Buffer& Buffer::operator=(const Buffer& buff)
{
	MutexLock lockLocal;
	MutexLock lock;
	if (mem_ != buff.mem_)
	{
		lockLocal.Lock(&mem_->Lock);
		--mem_->UseCount;
		if (mem_->UseCount == 0)
		{
			lockLocal.Unlock();
			MemoryManager::Instance()->FreeMemory(mem_);
		}
		lock.Lock(&buff.mem_->Lock);
		++buff.mem_->UseCount;
		mem_ = buff.mem_;
	}
	size_ = buff.size_;
	offset_ = buff.offset_;
	return *this;
}

Buffer::~Buffer()
{
	MutexLock lock(&mem_->Lock);
	--mem_->UseCount;
	if (mem_->UseCount == 0)
	{
		lock.Unlock();
		MemoryManager::Instance()->FreeMemory(mem_);
	}
	mem_ = nullptr;
}

Buffer Buffer::GetRegion(int size)
{
	return Buffer(mem_, offset_, size);
}

void* Buffer::GetData() const
{
	return (byte*)mem_->Data + offset_;
}

int32 Buffer::UseCount() const
{
	return mem_->UseCount;
}

void Buffer::Lock()
{
	mem_->Lock.Lock();
}

void Buffer::Unlock()
{
	mem_->Lock.Unlock();
}