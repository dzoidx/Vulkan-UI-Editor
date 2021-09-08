#pragma once
#include "Types.h"

template<typename T>
class Stack
{
public:
	Stack() :Stack(0) {}
	Stack(int32 initialCap) : top_(-1), capacity_(0), data_(0), default_()
	{
		if (initialCap > 0)
			EnsureSize(initialCap);
	}
	~Stack()
	{
		MemoryManager::Instance()->Free(data_, capacity_);
	}

	T& Peek()
	{
		if (top_ < 0)
			return default_;
		return *(data_ + top_);
	}
	T& Pop()
	{
		if (top_ < 0)
			return default_;
		return *(data_ + top_--);
	}
	void Push(T val)
	{
		EnsureSize(top_ + 2);
		++top_;
		data_[top_] = val;
	}
	int32 GetCount() const { return top_ + 1; }
private:
	void EnsureSize(int32 size)
	{
		if (data_ == nullptr)
		{
			data_ = MemoryManager::Instance()->Allocate<T>(size * sizeof(T));
			capacity_ = size;
			return;
		}
		if (capacity_ >= size)
			return;
		T* newArray = MemoryManager::Instance()->Allocate<T>(size * sizeof(T));
		memcpy(newArray, data_, sizeof(T) * (top_ + 1));
		MemoryManager::Instance()->Free(data_, capacity_);
		data_ = newArray;
		capacity_ = size;
	}
private:
	int32 top_;
	int32 capacity_;
	T* data_;
	T default_;
};