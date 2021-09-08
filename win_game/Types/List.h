#pragma once
#include "cassert"
#include "Types.h"
#include "Memory/MemoryManager.h"
#include "Utils/Numeric.h"

enum class ListCutResponse
{
	Ok,
	NegAmount,
	OverAmount
};

template<typename T>
class List
{
public:
	List() : List(1) {}
	List(const T* data, uint32 len)
	{
		m_data = (T*)MemoryManager::Instance()->AllocateRaw(sizeof(T) * len);
		for (uint32 i = 0; i < len; ++i)
		{
			new (&m_data[i]) T(*(data + i));
		}
		m_len = len;
		m_free = 0;
	}
	List(const List<T>& f)
	{
		m_data = (T*)MemoryManager::Instance()->AllocateRaw(f.m_len * sizeof(T));
		m_len = f.GetLen();
		m_free = 0;
		for (uint32 i = 0; i < m_len; ++i)
		{
			new (&m_data[i]) T(f.m_data[i]);
		}
	}
	List(List<T>&& f)
	{
		m_data = f.m_data;
		m_len = f.m_len;
		m_free = f.m_free;
		f.m_data = nullptr;
		f.m_len = 0;
		f.m_free = 0;
	}
	List<T>& operator=(const List<T>& l)
	{
		if (m_data == l.m_data)
			return *this;
		Clear();
		if (l.GetLen() > m_free)
		{
			EnsureExtraSize(l.GetLen());
		}
		for (uint32 i = 0; i < l.m_len; ++i)
		{
			new (&m_data[i]) T(l.m_data[i]);
		}
		m_len = l.m_len;
		m_free -= m_len;
		return *this;
	}
	List<T>& operator=(List<T>&& l)
	{
		assert(m_data != l.m_data);
		Clear();
		MemoryManager::Instance()->FreeRaw(m_data, sizeof(T) * m_free);
		m_data = l.m_data;
		m_len = l.m_len;
		m_free = l.m_free;
		l.m_data = nullptr;
		l.m_len = 0;
		l.m_free = 0;
		return *this;
	}
	~List()
	{
		Clear();
		MemoryManager::Instance()->FreeRaw(m_data, sizeof(T) * m_free);
	}
	List(uint32 cap)
	{
		m_data = cap ? (T*)MemoryManager::Instance()->AllocateRaw(sizeof(T) * cap) : nullptr;
		m_len = 0;
		m_free = cap;
	}
	void SetCapacity(uint32 cap)
	{
		if (m_len + m_free >= cap)
			return;
		EnsureExtraSize(cap - m_len);
	}
	// добавляет item в начало массива amount раз
	void PrependFill(uint32 amount, const T& item)
	{
		if (!amount) return;
		EnsureExtraSize(amount);
		for (int32 i = m_len - 1; i >= 0; --i)
		{
			uint32 offset = i + amount;
			if (offset >= m_len)
				new (&m_data[offset]) T((T&&)m_data[i]);
			else
				m_data[offset] = (T&&)m_data[i];
		}
		for (uint32 i = 0; i < amount; ++i)
		{
			m_data[i] = item;
		}
		m_free -= amount;
		m_len += amount;
	}
	void Add(T&& item)
	{
		EnsureExtraSize(1);
		new (&m_data[m_len]) T((T&&)item);
		++m_len;
		--m_free;
	}
	void Add(const T& item)
	{
		EnsureExtraSize(1);
		new (&m_data[m_len]) T(item);
		++m_len;
		--m_free;
	}
	void Add(List<T>& items)
	{
		EnsureExtraSize(items.GetLen());
		uint32 count = m_len + items.GetLen();
		for (uint32 i = m_len; i < count; ++i)
		{
			new (&m_data[i]) T(items[i - m_len]);
		}
		m_len += items.m_len;
		m_free -= items.m_len;
	}
	void Allocate(uint32 amount)
	{
		EnsureExtraSize(amount);
		m_free -= amount;
		m_len += amount;
	}
	T Last() const
	{
		assert(m_len > 0);
		return m_data[m_len - 1];
	}
	T& Last()
	{
		assert(m_len > 0);
		return m_data[m_len - 1];
	}
	void RemoveAt(uint32 offset, uint32 count)
	{
		assert(offset + count <= m_len);
		assert(m_len >= count);
		uint32 moveCount = m_len - count;
		for (uint32 i = offset; i < m_len; ++i)
		{
			if (i < moveCount)
				m_data[i] = (T&&)m_data[i + count];
			else
				m_data[i].~T();
		}
		m_len -= count;
		m_free += count;
	}
	T RemoveLast()
	{
		assert(m_len > 0);
		++m_free;
		return m_data[--m_len];
	}
	void RemoveLast(uint32 count)
	{
		assert(m_len >= count);
		m_free += count;
		m_len -= count;
	}
	void Clear()
	{
		if (m_data != nullptr)
		{
			for (uint32 i = 0; i < m_len; i++)
			{
				m_data[i].~T();
			}
		}
		m_free += m_len;
		m_len = 0;
	}
	// метод безопасно использовать только для простых типов, т.к. используется memset
	void ZeroFill(uint32 amount)
	{
		EnsureExtraSize(amount);
		memset(&m_data[m_len], 0, sizeof(T) * amount);
		m_free -= amount;
		m_len += amount;
	}
	void Fill(uint32 amount, const T& value)
	{
		EnsureExtraSize(amount);
		for (uint32 i = 0; i < amount; ++i)
		{
			new (&m_data[m_len + i]) T(value);
		}
		m_free -= amount;
		m_len += amount;
	}
	ListCutResponse Cut(int32 amount)
	{
		if (amount == 0)
			return ListCutResponse::Ok;
		if (amount < 1)
			return ListCutResponse::NegAmount;
		if (amount > m_len)
			return ListCutResponse::OverAmount;

		uint32 offset = m_len - amount;
		for (uint32 i = 0; i < amount; ++i)
		{
			m_data[offset + i].~T();
		}

		m_free += amount;
		m_len -= amount;
		return ListCutResponse::Ok;
	}
	uint32 GetLen() const { return m_len; }
	byte* GetBuffer() const { return (byte*)m_data; }
	T* GetData() const { return m_data; }
	inline T& operator[](uint32 idx)
	{
		assert(idx < m_len);
		return m_data[idx];
	}
	inline const T& operator[](uint32 idx) const
	{
		assert(idx < m_len);
		return m_data[idx];
	}
private:
	void EnsureExtraSize(uint32 size)
	{
		if (m_free >= size)
			return;
		uint32 newSize = ToPowerOfTwo(m_len + m_free + size);
		T* buff = (T*)MemoryManager::Instance()->AllocateRaw(sizeof(T) * newSize);
		uint32 oldFree = m_free;
		m_free = newSize - m_len;
		memset(&buff[m_len], 0, sizeof(T) * m_free);
		for (uint32 i = 0; i < m_len; ++i)
		{
			new (&buff[i]) T((T&&)m_data[i]);
			m_data[i].~T();
		}
		if (m_data != nullptr)
			MemoryManager::Instance()->FreeRaw(m_data, sizeof(T) * oldFree);
		m_data = buff;
	}
private:
	T* m_data;
	uint32 m_len;
	uint32 m_free;
};