#pragma once
#include "State.h"
#include "Types.h"

inline int32 MostSignedBit(uint64 n)
{
	int32 result = 0;
	while (n > 0)
	{
		n >>= 1;
		++result;
	}
	return result;
}

inline uint64 ToPowerOfTwo(uint64 n)
{
    int32 c = MostSignedBit(n);
	assert(c < 64);
	return 1 << c;
}

inline uint32 RandomUInt32()
{
	return GetCurrentGameSate()->Seed = (1664525 * GetCurrentGameSate()->Seed + 1013904223) % 4294967296;
}

inline void RandomBytes(byte* data, uint32 len)
{
	for (uint32 i = 0; i < len; ++i)
	{
		*data++ = (byte)RandomUInt32();
	}
}

inline bool IsPowerOfTwo(uint64 n)
{
	uint32 bits = 0;
    while (n > 0 && bits < 2)
    {
        bits += n & 1;
		n >>= 1;
    }
	return bits == 1;
}