#pragma once
#include "Types.h"

inline bool isBigEndian()
{
	int32 n = 1;
	byte* p = (byte*)&n;

	return *p == 0;
}

inline bool isLittleEndian()
{
	return !isBigEndian();
}

#define SWAPS(n) ((n & 0xff) << 8 | (n >> 8))
#define SWAPL(n) ((n & 0xff) << 24 | (((n >> 8) & 0xff) << 16) | (((n >> 16) & 0xff) << 8) | ((n >> 24) & 0xff))
#define HTONS(n) (isBigEndian() ? n : SWAPS(n))
#define HTONL(n) (isBigEndian() ? n : SWAPL(n))
#define NTOHS(n) (isBigEndian() ? n : SWAPS(n))
#define NTOHL(n) (isBigEndian() ? n : SWAPL(n))