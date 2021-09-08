#include "Murmur32.h"

uint32 HashMurmur32(const byte* key, uint64 size, uint32 seed)
{
	uint32 h = seed;
	if (size > 3)
	{
		uint64 i = size >> 2;
		do
		{
			uint32 k = *((uint32*)key);
			key += sizeof(uint32);
			k *= 0xcc9e2d51;
			k = (k << 15) | (k >> 17);
			k *= 0x1b873593;
			h ^= k;
			h = (h << 13) | (h >> 19);
			h = h * 5 + 0xe6546b64;
		} while (--i);
	}
	if (size & 3)
	{
		uint64 i = size & 3;
		uint32 k = 0;
		do
		{
			k <<= 8;
			k |= key[i - 1];
		} while (--i);
		k *= 0xcc9e2d51;
		k = (k << 15) | (k >> 17);
		k *= 0x1b873593;
		h ^= k;
	}
	h ^= size;
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return h;
}