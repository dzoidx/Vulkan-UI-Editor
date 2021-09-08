#pragma once
#include <cassert>
#include "Types.h"
#include "Memory/MemoryManager.h"

template<typename Key>
class DefaultHash
{
public:
	uint32 GetHash(const Key& k) const
	{
		return HashMurmur32((byte*)&k, sizeof(Key), 0);
	}
};

template<typename Key>
class SimpleHash
{
public:
	uint32 GetHash(const Key& k) const
	{
		assert(sizeof(k) <= 4);
		return (uint32)k;
	}
};

template<typename Key>
class DefaultHashArray
{
public:
	uint32 GetHash(const Key* k, uint32 count) const
	{
		return HashMurmur32((byte*)k, count * sizeof(Key), 0);
	}
};

template<typename kT, typename vT>
struct HashTableEntryT
{
	kT Key;
	vT Value;
	uint32 Hash;
	int32 Next;
	bool Valid;
};

template<typename kT, typename vT, class Hash = DefaultHash<kT>>
class HashTableT
{
public:
	HashTableT()
		:HashTableT(1)
	{}
	HashTableT(int32 capacity)
	{
		int32 count = capacity < 1 ? 3 : capacity;
		Buckets = MemoryManager::Instance()->Allocate<int32>(count);
		int32 zeroCount = count;
		while (zeroCount--)
		{
			Buckets[zeroCount] = -1;
		}
		Entries = (HashTableEntryT<kT, vT>*)MemoryManager::Instance()->AllocateRaw(count * sizeof(HashTableEntryT<kT, vT>));
		FreeEntryIndex = -1;
		Free = 0;
		Size = count;
		Count = 0;
	}
	~HashTableT()
	{
		Clear();
		MemoryManager::Instance()->Free<int32>(Buckets, Size);
		MemoryManager::Instance()->FreeRaw(Entries, sizeof(HashTableEntryT<kT, vT>) * Size);
		Buckets = nullptr;
		Size = 0;
	}

	bool InsertCopy(kT key, vT value, bool addOnly)
	{
		uint32 hash = hash_.GetHash(key);
		uint32 bucketIndex = hash % Size;
		uint32 sameKeyEntries = 0;
		for (int32 i = Buckets[bucketIndex]; i >= 0; i = Entries[i].Next)
		{
			if (Entries[i].Hash == hash && Entries[i].Key == key)
			{
				if (addOnly)
					return false;
				Entries[i].Value = value;
				return true;
			}
			++sameKeyEntries;
		}

		int32 entryIndex;
		if (Free > 0)
		{
			entryIndex = FreeEntryIndex;
			FreeEntryIndex = Entries[entryIndex].Next;
			--Free;
		}
		else
		{
			if (Count == Size)
			{
				Resize(Size << 1);
				bucketIndex = hash % Size;
			}
			entryIndex = Count;
			++Count;
		}
		Entries[entryIndex].Hash = hash;
		Entries[entryIndex].Key = key;
		new (&Entries[entryIndex].Value) vT(value);
		Entries[entryIndex].Next = Buckets[bucketIndex];
		Entries[entryIndex].Valid = true;
		Buckets[bucketIndex] = entryIndex;
		if (sameKeyEntries <= 100)
			return true;
		Resize(Size << 1);
		return true;
	}
	bool Insert(kT key, vT&& value, bool addOnly)
	{
		uint32 hash = hash_.GetHash(key);
		uint32 bucketIndex = hash % Size;
		uint32 sameKeyEntries = 0;
		for (int32 i = Buckets[bucketIndex]; i >= 0; i = Entries[i].Next)
		{
			if (Entries[i].Hash == hash && Entries[i].Key == key)
			{
				if (addOnly)
					return false;
				Entries[i].Value = (vT&&)value;
				return true;
			}
			++sameKeyEntries;
		}

		int32 entryIndex;
		if (Free > 0)
		{
			entryIndex = FreeEntryIndex;
			FreeEntryIndex = Entries[entryIndex].Next;
			--Free;
		}
		else
		{
			if (Count == Size)
			{
				Resize(Size << 1);
				bucketIndex = hash % Size;
			}
			entryIndex = Count;
			++Count;
		}
		Entries[entryIndex].Hash = hash;
		Entries[entryIndex].Key = key;
		new (&Entries[entryIndex].Value) vT((vT&&)value);
		Entries[entryIndex].Next = Buckets[bucketIndex];
		Entries[entryIndex].Valid = true;
		Buckets[bucketIndex] = entryIndex;
		if (sameKeyEntries <= 100)
			return true;
		Resize(Size << 1);
		return true;
	}
	void Clear()
	{
		if (Count <= 0) return;
		for (int32 i = 0; i < Size; ++i)
		{
			Buckets[i] = -1;
			if (Entries[i].Valid)
			{
				Entries[i].Value.~vT();
				Entries[i].Valid = false;
			}
		}
		FreeEntryIndex = -1;
		Free = 0;
		Count = 0;
	}
	void Remove(kT key)
	{
		if (Buckets == nullptr)
			return;
		uint32 hash = hash_.GetHash(key);
		int32 bucketIndex = hash % Size;
		int32 prevEntry = -1;
		for (int32 pos = Buckets[bucketIndex]; pos >= 0; pos = Entries[pos].Next)
		{
			if (hash == Entries[pos].Hash && Entries[pos].Key == key)
			{
				if (prevEntry < 0)
					Buckets[bucketIndex] = Entries[pos].Next;
				else
					Entries[prevEntry].Next = Entries[pos].Next;
				if (Entries[pos].Valid)
					Entries[pos].Value.~vT();
				Entries[pos].Valid = false;
				Entries[pos].Next = FreeEntryIndex;
				FreeEntryIndex = pos;
				++Free;
				return;
			}
			prevEntry = pos;
		}
	}
	vT& Find(const kT& key)
	{
		uint32 hash = hash_.GetHash(key);
		int32 bucketIndex = hash % Size;
		for (int32 pos = Buckets[bucketIndex]; pos >= 0; pos = Entries[pos].Next)
		{
			if (Entries[pos].Hash == hash && Entries[pos].Key == key)
			{
				return Entries[pos].Value;
			}
		}
		return default_;
	}
	vT& At(int32 idx, bool& is_valid)
	{
		is_valid = Entries[idx].Valid;
		return Entries[idx].Value;
	}
	int32 GetCount() const
	{
		return Count;
	}

private:
	void Resize(int32 newSize)
	{
		int32* buckets = MemoryManager::Instance()->Allocate<int32>(newSize);
		for (int32 i = 0; i < newSize; ++i)
			buckets[i] = -1;
		HashTableEntryT<kT, vT>* entries = (HashTableEntryT<kT, vT>*)MemoryManager::Instance()->AllocateRaw(newSize * sizeof(HashTableEntryT<kT, vT>));
		memcpy(entries, Entries, sizeof(HashTableEntryT<kT, vT>) * Count);
		for (int32 i = 0; i < Count; ++i)
		{
			if (Entries[i].Valid)
			{
				uint32 bucketIndex = Entries[i].Hash % newSize;
				entries[i].Key = Entries[i].Key;
				entries[i].Next = buckets[bucketIndex];
				entries[i].Valid = true;
				entries[i].Hash = Entries[i].Hash;
				new (&entries[i].Value) vT((vT&&)Entries[i].Value);
				buckets[bucketIndex] = i;
			}
		}
		MemoryManager::Instance()->Free<int32>(Buckets, Size);
		MemoryManager::Instance()->FreeRaw(Entries, Size * sizeof(HashTableEntryT<kT, vT>));
		Buckets = buckets;
		Entries = entries;
		Size = newSize;
	}

private:
	Hash hash_;
	int32* Buckets;
	HashTableEntryT<kT, vT>* Entries;
	int32 Size;
	int32 Count;
	int32 Free;
	int32 FreeEntryIndex;
	vT default_;
};

template<typename kT, typename vT>
struct HashTableEntryTArray
{
	kT* Key;
	int32 KeyLen;
	vT Value;
	uint32 Hash;
	int32 Next;
	bool Valid;
};

/*
Хештаблица где ключом является массив объектов типа T (например char*)
Это единственное отличие от HashTableT
*/
template<typename kT, typename vT>
class HashTableTArray
{
public:
	HashTableTArray()
		:HashTableTArray(1)
	{}
	HashTableTArray(int32 capacity)
	{
		int32 count = capacity < 1 ? 3 : capacity;
		Buckets = MemoryManager::Instance()->Allocate<int32>(count);
		int32 zeroCount = count;
		while (zeroCount--)
		{
			Buckets[zeroCount] = -1;
		}
		Entries = (HashTableEntryTArray<kT, vT>*)MemoryManager::Instance()->AllocateRaw(count * sizeof(HashTableEntryTArray<kT, vT>));
		FreeEntryIndex = -1;
		Free = 0;
		Size = count;
		Count = 0;
	}
	~HashTableTArray()
	{
		Clear();
		MemoryManager::Instance()->Free<int32>(Buckets, Size);
		MemoryManager::Instance()->FreeRaw(Entries, Size * sizeof(HashTableEntryTArray<kT, vT>));
		Buckets = nullptr;
		Entries = nullptr;
		Size = 0;
	}

	bool Insert(kT* key, int32 len, vT value, bool addOnly)
	{
		uint32 hash = HashMurmur32((byte*)key, sizeof(kT) * len, 0);
		uint32 bucketIndex = hash % Size;
		uint32 sameKeyEntries = 0;
		for (int32 i = Buckets[bucketIndex]; i >= 0; i = Entries[i].Next)
		{
			if (Entries[i].Hash == hash && KeyEquals(key, len, Entries[i]))
			{
				if (addOnly)
					return false;
				Entries[i].Value = value;
				return true;
			}
			++sameKeyEntries;
		}

		int32 entryIndex;
		if (Free > 0)
		{
			entryIndex = FreeEntryIndex;
			FreeEntryIndex = Entries[entryIndex].Next;
			--Free;
		}
		else
		{
			if (Count == Size)
			{
				Resize(Size << 1);
				bucketIndex = hash % Size;
			}
			entryIndex = Count;
			++Count;
		}
		Entries[entryIndex].Hash = hash;
		Entries[entryIndex].Key = key;
		Entries[entryIndex].KeyLen = len;
		new (&Entries[entryIndex].Value) vT(value);
		Entries[entryIndex].Next = Buckets[bucketIndex];
		Entries[entryIndex].Valid = true;
		Buckets[bucketIndex] = entryIndex;
		if (sameKeyEntries <= 100)
			return true;
		Resize(Size << 1);
		return true;
	}
	bool Insert(kT* key, int32 len, vT&& value, bool addOnly)
	{
		uint32 hash = HashMurmur32((byte*)key, sizeof(kT) * len, 0);
		uint32 bucketIndex = hash % Size;
		uint32 sameKeyEntries = 0;
		for (int32 i = Buckets[bucketIndex]; i >= 0; i = Entries[i].Next)
		{
			if (Entries[i].Hash == hash && KeyEquals(key, len, Entries[i]))
			{
				if (addOnly)
					return false;
				Entries[i].Value = (vT&&)value;
				return true;
			}
			++sameKeyEntries;
		}

		int32 entryIndex;
		if (Free > 0)
		{
			entryIndex = FreeEntryIndex;
			FreeEntryIndex = Entries[entryIndex].Next;
			--Free;
		}
		else
		{
			if (Count == Size)
			{
				Resize(Size << 1);
				bucketIndex = hash % Size;
			}
			entryIndex = Count;
			++Count;
		}
		Entries[entryIndex].Hash = hash;
		Entries[entryIndex].Key = key;
		Entries[entryIndex].KeyLen = len;
		new (&Entries[entryIndex].Value) vT((vT&&)value);
		Entries[entryIndex].Next = Buckets[bucketIndex];
		Entries[entryIndex].Valid = true;
		Buckets[bucketIndex] = entryIndex;
		if (sameKeyEntries <= 100)
			return true;
		Resize(Size << 1);
		return true;
	}
	void Clear()
	{
		if (Count <= 0) return;
		for (int32 i = 0; i < Size; ++i)
		{
			Buckets[i] = -1;
			if (Entries[i].Valid)
			{
				Entries[i].Value.~vT();
				Entries[i].Valid = false;
			}
		}
		FreeEntryIndex = -1;
		Free = 0;
		Count = 0;
	}
	void Remove(kT* key, int32 len)
	{
		if (Buckets == nullptr)
			return;
		uint32 hash = HashMurmur32((byte*)key, sizeof(kT) * len, 0);
		int32 bucketIndex = hash % Size;
		int32 prevEntry = -1;
		for (int32 pos = Buckets[bucketIndex]; pos >= 0; pos = Entries[pos].Next)
		{
			if (hash == Entries[pos].Hash && KeyEquals(key, len, Entries[pos]))
			{
				if (prevEntry < 0)
					Buckets[bucketIndex] = Entries[pos].Next;
				else
					Entries[prevEntry].Next = Entries[pos].Next;
				if (Entries[pos].Valid)
					Entries[pos].Value.~vT();
				Entries[pos].Valid = false;
				Entries[pos].Next = FreeEntryIndex;
				FreeEntryIndex = pos;
				++Free;
				return;
			}
			prevEntry = pos;
		}
	}

	bool HasKey(kT* key, int32 len)
	{
		if (Buckets == nullptr)
			return;
		uint32 hash = HashMurmur32((byte*)key, sizeof(kT) * len, 0);
		int32 bucketIndex = hash % Size;
		for (int32 pos = Buckets[bucketIndex]; pos >= 0; pos = Entries[pos].Next)
		{
			if (Entries[pos].Hash == hash && KeyEquals(key, len, Entries[pos]))
			{
				return true;
			}
		}
	}

	vT& Find(kT* key, int32 len)
	{
		if (Buckets == nullptr)
			return nullptr;
		uint32 hash = HashMurmur32((byte*)key, sizeof(kT) * len, 0);
		int32 bucketIndex = hash % Size;
		for (int32 pos = Buckets[bucketIndex]; pos >= 0; pos = Entries[pos].Next)
		{
			if (Entries[pos].Hash == hash && KeyEquals(key, len, Entries[pos]))
			{
				return Entries[pos].Value;
			}
		}
		return vT{};
	}
	vT& At(int32 idx, bool& is_valid)
	{
		is_valid = Entries[idx].Valid;
		return Entries[idx].Value;
	}
	int32 GetCount() const
	{
		return Count;
	}

private:
	bool KeyEquals(kT* key, int32 len, HashTableEntryTArray<kT, vT>& entry)
	{
		if (!entry.Valid)
			return false;
		if (len != entry.KeyLen)
			return false;

		kT* l = key;
		kT* r = entry.Key;
		while (len > 0 && *l == *r)
		{
			++l;
			++r;
			--len;
		}
		return len == 0;
	}
	void Resize(int32 newSize)
	{
		int32* buckets = MemoryManager::Instance()->Allocate<int32>(newSize);
		for (int32 i = 0; i < newSize; ++i)
			buckets[i] = -1;
		HashTableEntryTArray<kT, vT>* entries = (HashTableEntryTArray<kT, vT>*)MemoryManager::AllocateRaw(newSize * sizeof(HashTableEntryTArray<kT, vT>));
		for (int32 i = 0; i < Count; ++i)
		{
			if (Entries[i].Valid)
			{
				uint32 bucketIndex = Entries[i].Hash % newSize;
				entries[i].Key = Entries[i].Key;
				entries[i].KeyLen = Entries[i].KeyLen;
				entries[i].Next = buckets[bucketIndex];
				entries[i].Valid = true;
				entries[i].Hash = Entries[i].Hash;
				new (&entries[i].Value) vT((vT&&)Entries[i].Value);
				buckets[bucketIndex] = i;
			}
		}
		MemoryManager::Instance()->Free<int32>(Buckets);
		MemoryManager::Instance()->FreeRaw(Entries);
		Buckets = buckets;
		Entries = entries;
		Size = newSize;
	}

private:
	int32* Buckets;
	HashTableEntryTArray<kT, vT>* Entries;
	int32 Size;
	int32 Count;
	int32 Free;
	int32 FreeEntryIndex;
};
