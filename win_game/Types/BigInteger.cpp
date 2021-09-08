#include "BigInteger.h"

#include "Memory/TempMemory.h"


inline void BigInteger_SumList(List<uint32>& destList, uint32 offset, uint64 add)
{
	uint32* dest = destList.GetData();
	uint64 accum = dest[offset] + (uint32)add;
	uint32 carry = (uint32)(accum > 0xFFFFFFFF);
	dest[offset++] = (uint32)accum;
	accum = dest[offset] + (uint32)(add >> 32) + carry;
	dest[offset++] = (uint32)accum;
	while (accum > 0xFFFFFFFF)
	{
		accum = dest[offset] + 1;
		dest[offset++] = (uint32)accum;
	}
	assert(offset <= destList.GetLen());
}

inline void BigInteger_SumList(uint32* dest, uint32 len, uint32 offset, uint64 add)
{
	uint64 accum = (uint64)dest[offset] + (uint32)add;
	uint32 carry = (uint32)(accum > 0xFFFFFFFF);
	dest[offset++] = (uint32)accum;
	accum = (uint64)dest[offset] + (uint32)(add >> 32) + carry;
	dest[offset++] = (uint32)accum;
	while (accum > 0xFFFFFFFF)
	{
		accum = (uint64)dest[offset] + 1;
		dest[offset++] = (uint32)accum;
	}
	assert(offset <= len);
}

inline void BigInteger_Trim(List<uint32>& arr)
{
	uint32 len = arr.GetLen();
	int32 mlen = len - 1;
	uint32* data = arr.GetData() + mlen;
	int32 count = 0;
	while (*data == 0 && mlen > count)
	{
		--data;
		++count;
	}
	arr.RemoveLast((uint32)count);
}

inline void BigInteger_Sum(List<uint32>& num1, const List<uint32>& num2)
{
	uint32 len1 = num1.GetLen();
	uint32 len2 = num2.GetLen();
	uint32* data2 = num2.GetData();
	uint32 carry = 0;
	uint64 accum;

	uint32 maxLen = len1 > len2 ? len1 : len2;
	uint32 sharedLen = len1 < len2 ? len1 : len2;
	num1.SetCapacity(maxLen);
	uint32* data1 = num1.GetData();

	for (uint32 i = 0; i < sharedLen; ++i)
	{
		accum = (uint64)data1[i] + data2[i] + carry;
		data1[i] = (uint32)accum;
		carry = (uint32)(accum >> 32);
	}

	if (len1 > sharedLen)
	{
		uint32 i = sharedLen;
		while (i < len1 && carry)
		{
			accum = (uint64)data1[i] + carry;
			data1[i++] = (uint32)accum;
			carry = (uint32)(accum >> 32);
		}
	}
	else
	{
		uint32 i = sharedLen;
		while (i < len2)
		{
			accum = (uint64)data2[i++] + carry;
			num1.Add((uint32)accum);
			carry = (uint32)(accum >> 32);
		}
	}
	if (carry)
		num1.Add(carry);

	BigInteger_Trim(num1);
}

inline void BigInteger_Sum(const List<uint32>& num1, const List<uint32>& num2, List<uint32>& result)
{
	assert(&num1 != &result);
	assert(&num2 != &result);
	uint32 len1 = num1.GetLen();
	uint32 len2 = num2.GetLen();
	uint32 lenRes = result.GetLen();
	uint32* data1 = num1.GetData();
	uint32* data2 = num2.GetData();
	uint32 carry = 0;
	uint64 accum;

	uint32 maxLen = len1 > len2 ? len1 : len2;
	uint32 sharedLen = len1 < len2 ? len1 : len2;
	if (sharedLen > lenRes)
	{
		result.SetCapacity(maxLen + 1);
		lenRes = maxLen + 1;
	}
	result.Clear();
	result.Fill(lenRes, 0);
	uint32* resultData = result.GetData();

	for (uint32 i = 0; i < sharedLen; ++i)
	{
		accum = (uint64)data1[i] + data2[i] + carry;
		resultData[i] = (uint32)accum;
		carry = (uint32)(accum >> 32);
	}

	uint32 offset = sharedLen;
	uint32* numX = len1 > sharedLen ? data1 : data2;
	while (carry)
	{
		accum = (uint64)numX[offset] + carry;
		resultData[offset++] = (uint32)accum;
		carry = (uint32)(accum >> 32);
	}
	BigInteger_Trim(result);
}

inline void BigInteger_MultiplePrecisionMulLimit(const List<uint32>& x, const List<uint32>& y, List<uint32>& result, uint32 limit)
{
	uint64 accum;
	uint64 carry = 0;
	uint32 xLen = x.GetLen();
	uint32 yLen = y.GetLen();
	xLen = xLen > limit ? limit : xLen;
	uint32 resultLen = xLen + yLen;
	uint32* xData = x.GetData();
	uint32* yData = y.GetData();
	result.SetCapacity(resultLen);
	result.Clear();
	result.ZeroFill(resultLen);
	uint32* resultData = result.GetData();
	//memset(resultData, 0, sizeof(uint32) * resultLen);

	for (uint32 i = 0; i < xLen; ++i)
	{
		carry = 0;
		uint32 jLen = limit - i;
		jLen = jLen > yLen ? yLen : jLen;
		for (uint32 j = 0; j < jLen; ++j)
		{
			uint32 pos = i + j;
			accum = (uint64)resultData[pos] + (uint64)xData[i] * (uint64)yData[j] + carry;
			resultData[pos] = (uint32)accum;
			carry = accum >> 32;
		}
		if (i + jLen < limit)
			resultData[i + jLen] += (uint32)carry;
	}
	BigInteger_Trim(result);
}

inline void BigInteger_MultiplePrecisionMul(const List<uint32>& x, uint32 y, List<uint32>& result)
{
	if (y == 0)
	{
		result.Clear();
		result.Add(0);
		return;
	}
	if (y == 1)
	{
		result = x;
		return;
	}
	uint64 accum;
	uint32 xLen = x.GetLen();
	uint32 resultLen = xLen + 1;
	uint32* xData = x.GetData();
	result.SetCapacity(resultLen);
	result.Clear();
	result.Fill(resultLen, 0);
	uint32* resultData = result.GetData();

	for (uint32 i = 0; i < xLen; ++i)
	{
		accum = (uint64)resultData[i] + (uint64)xData[i] * y;
		resultData[i] = (uint32)accum;
		resultData[i + 1] = (uint32)(accum >> 32);
	}
	BigInteger_Trim(result);
}

inline void BigInteger_MultiplePrecisionMulOffset(const List<uint32>& x, const List<uint32>& y, List<uint32>& result, uint32 xOffset, uint32 yOffset)
{
	uint64 accum;
	uint64 carry;
	uint32 xLen = x.GetLen() - xOffset;
	uint32 yLen = y.GetLen() - yOffset;
	uint32 resultLen = xLen + yLen;
	uint32* xData = x.GetData();
	uint32* yData = y.GetData();
	result.SetCapacity(resultLen);
	result.Clear();
	result.Fill(resultLen, 0); // TODO: отказаться от листа
	uint32* resultData = result.GetData();
	//memset(resultData, 0, sizeof(uint32) * resultLen);

	for (uint32 i = 0; i < xLen; ++i)
	{
		carry = 0;
		for (uint32 j = 0; j < yLen; ++j)
		{
			uint32 pos = i + j;
			accum = (uint64)resultData[pos] + (uint64)xData[i + xOffset] * (uint64)yData[j + yOffset] + carry;
			resultData[pos] = (uint32)accum;
			carry = accum >> 32;
		}
		resultData[i + yLen] = (uint32)carry;
	}
	BigInteger_Trim(result);
}

inline void BigInteger_MultiplePrecisionMulOffset(const List<uint32>& x, const List<uint32>& y, List<uint32>& result, uint32 offset)
{
	uint64 accum;
	uint64 carry;
	uint32 xLen = x.GetLen();
	uint32 yLen = y.GetLen();
	uint32 resultLen = xLen + yLen;
	uint32* xData = x.GetData();
	uint32* yData = y.GetData();
	result.SetCapacity(resultLen);
	result.Clear();
	result.ZeroFill(resultLen);
	uint32* resultData = result.GetData();
	//memset(resultData, 0, sizeof(uint32) * resultLen);

	for (uint32 i = offset; i < xLen; ++i)
	{
		carry = 0;
		for (uint32 j = offset; j < yLen; ++j)
		{
			uint32 pos = i + j;
			accum = (uint64)resultData[pos] + (uint64)xData[i] * (uint64)yData[j] + carry;
			resultData[pos] = (uint32)accum;
			carry = accum >> 32;
		}
		resultData[i + yLen] = (uint32)carry;
	}
	BigInteger_Trim(result);
}

// Оптимизация части Barrett reduction как описано в книге
// берем от x только k - 1 разрядов (смещение)
// поразрядно умножаем на y, только те разряды что будут в результате последней операции деления + 2 разряда чтоб сохранить точность при переносе
// (x/b^k-1 * y) / b^k+1
inline void BigInteger_MultiplePrecisionDivMulDiv(const List<uint32>& x, const List<uint32>& y, List<uint32>& result, uint32 k)
{
	uint64 accum;
	uint64 carry;
	uint32 xOffset = k - 1;
	uint32 rOffset = xOffset * 2;
	uint32 xLen = x.GetLen();
	uint32 yLen = y.GetLen();
	uint32 resultLen = xLen + yLen;
	uint32* xData = x.GetData();
	uint32* yData = y.GetData();
	result.SetCapacity(resultLen);
	result.Clear();
	result.ZeroFill(resultLen);
	uint32* resultData = result.GetData();
	//memset(resultData, 0, sizeof(uint32) * resultLen);

	for (uint32 i = xOffset; i < xLen; ++i)
	{
		carry = 0;
		uint32 jOffset = rOffset >= i ? rOffset - i : 0;
		for (uint32 j = jOffset; j < yLen; ++j)
		{
			uint32 pos = i + j;
			int32 rPos = pos - rOffset;
			assert(rPos > -1);
			accum = (uint64)resultData[rPos] + (uint64)xData[i] * (uint64)yData[j] + carry;
			assert(rPos < (int32)resultLen);
			resultData[rPos] = (uint32)accum;
			carry = accum >> 32;
		}
		assert(i + yLen - rOffset < resultLen);
		resultData[i + yLen - rOffset] = (uint32)carry;
	}
	BigInteger_Trim(result);
	result.RemoveAt(0, 2);
}

inline void BigInteger_Square(const List<uint32>& num, List<uint32>& result)
{
	uint64 accum;
	uint64 accum2;
	uint64 carry;
	uint64 carry64;
	uint32 len = num.GetLen();
	result.Clear();
	result.SetCapacity(len * 2 + 1);
	result.ZeroFill(len * 2 + 1);
	uint32* rData = result.GetData();
	uint32* data = num.GetData();
	for (uint32 i = 0; i < len; ++i)
	{
		accum = (uint64)rData[i * 2] + (uint64)data[i] * (uint64)data[i];
		rData[i * 2] = (uint32)accum;
		carry = (uint32)(accum >> 32);
		for (uint32 j = i + 1; j < len; ++j)
		{
			accum = (uint64)data[j] * (uint64)data[i];
			carry64 = (accum & 0x8000000000000000) != 0;
			accum *= 2;
			accum2 = accum;
			accum += (uint64)rData[i + j] + carry;
			carry64 |= accum < accum2 ? 1 : 0;
			rData[i + j] = (uint32)accum;
			carry = (uint32)(accum >> 32) | (carry64 << 32);
		}
		rData[i + len] += (uint32)carry;
		rData[i + len + 1] = (uint32)(carry >> 32);
	}
	BigInteger_Trim(result);
}

BigInteger::BigInteger(uint32 num)
	:parts_()
{
	parts_.Add(num);
}

BigInteger::BigInteger(const BigInteger& num)
	: parts_(num.parts_)
{
}

BigInteger::BigInteger(const uint32* data, uint32 len)
	: parts_(data, len)
{
	BigInteger_Trim(parts_);
}

BigInteger::BigInteger(const byte* data, uint32 len)
	: parts_(len / 4 + 1)
{
	uint32 parts = len / 4;
	uint32 part;
	for (uint32 i = 0; i < parts; ++i)
	{
		part = *data++;
		part += (*data++) << 8;
		part += (*data++) << 16;
		part += (*data++) << 24;
		parts_.Add(part);
	}
	part = 0;
	len -= parts * 4;
	if (len > 0)
	{
		part += *data++;
	}
	if (len > 1)
	{
		part += *data++ << 8;
	}
	if (len > 2)
	{
		part += *data++ << 16;
	}
	if (part)
		parts_.Add(part);
}

BigInteger::BigInteger(const List<uint32>& parts)
	:parts_(parts)
{}
BigInteger::BigInteger(List<uint32>&& parts)
	: parts_((List<uint32>&&)parts)
{}

BigInteger BigInteger::Pow(const BigInteger& num, const BigInteger& pow)
{
	BigInteger result(1);
	BigInteger e(pow);
	BigInteger n(num);
	while (!e.IsZero())
	{
		if (e.parts_[0] & 1) result *= n;
		e.ShiftRight();
		if (!e.IsZero()) n *= n;
	}
	return result;
}

BigInteger BigInteger::Square(const BigInteger& num)
{
	BigInteger result(0);
	BigInteger_Square(num.parts_, result.parts_);
	return result;
}

// mod - 14.42 Algorithm Barrett modular reduction (handbook of applied cryptography p.603)
// pow - 14.79 Algorithm Left-to-right binary exponentiation (handbook of applied cryptography p.615)
BigInteger BigInteger::PowMod(const BigInteger& num, const BigInteger& pow, const BigInteger& mod)
{
	BigInteger r(1);
	uint32 k = mod.parts_.GetLen();
	uint32 k2 = k + 1;
	r.ShiftLeft(k * 2 * 32);
	r /= mod;
	BigInteger q2(0);
	BigInteger r1(0);
	BigInteger r2(0);
	BigInteger add(1);
	BigInteger e(pow);
	BigInteger nBuff1(num);
	BigInteger nBuff2(num);
	BigInteger* n = &nBuff2;
	BigInteger* nBuff = &nBuff1;
	BigInteger* nBuffs[] = { &nBuff1, &nBuff2 };
	uint32 nStep = 0;
	add.ShiftLeft((k + 1) * 32);

	BigInteger resultBuff1(1);
	BigInteger resultBuff2(0);
	BigInteger* result = &resultBuff2;
	BigInteger* resultBuff = &resultBuff1;
	BigInteger* resultBuffs[] = { &resultBuff1, &resultBuff2 };
	uint32 rStep = 0;

	q2.parts_.SetCapacity(k * 3);
	r1.parts_.SetCapacity(k);
	r2.parts_.SetCapacity(k);
	nBuff1.parts_.SetCapacity(k * 3);
	nBuff2.parts_.SetCapacity(k * 3);
	resultBuff1.parts_.SetCapacity(k * 3);
	resultBuff2.parts_.SetCapacity(k * 3);
	while (!e.IsZero())
	{
		if (e.parts_[0] & 1)
		{
			BigInteger_MultiplePrecisionMulOffset(resultBuff->parts_, nBuff->parts_, result->parts_, 0);
			//result *= n;
			if (*result > mod)
			{
				BigInteger_MultiplePrecisionDivMulDiv(result->parts_, r.parts_, q2.parts_, k);
				r1 = *result;
				CutHeadBits(r1, (k + 1) * 32);
				//r2 = q2 * mod;
				BigInteger_MultiplePrecisionMulLimit(q2.parts_, mod.parts_, r2.parts_, k + 1);
				if (r1 < r2)
				{
					r1 += add;
					r1 -= r2;
				}
				else
				{
					r1 -= r2;
				}
				r1.Mod(mod);
				/*
				if (r1 > mod) r1 -= mod; // 14.44 Note ii (partial justiﬁcation of correctness of Barrett reduction)
				if (r1 > mod) r1 -= mod;
				assert(r1 < mod);*/
				*result = r1;
			}
			result = resultBuffs[rStep];
			rStep = (rStep + 1) & 1;
			resultBuff = resultBuffs[rStep];
		}
		e.ShiftRight();
		if (!e.IsZero())
		{
			BigInteger_Square(nBuff->parts_, n->parts_);
			if (*n > mod)
			{
				BigInteger_MultiplePrecisionDivMulDiv(n->parts_, r.parts_, q2.parts_, k);

				r1 = *n;
				CutHeadBits(r1, (k + 1) * 32);
				//r2 = q2 * mod;
				BigInteger_MultiplePrecisionMulLimit(q2.parts_, mod.parts_, r2.parts_, k + 1);
				if (r1 < r2)
				{
					r1 += add;
					r1 -= r2;
				}
				else
				{
					r1 -= r2;
				}
				r1.Mod(mod);
				/*
				if (r3 > mod) r1 -= mod; // 14.44 Note ii (partial justiﬁcation of correctness of Barrett reduction)
				if (r3 > mod) r1 -= mod;
				assert(r1 < mod);*/
				*n = r1;
			}
			n = nBuffs[nStep];
			nStep = (nStep + 1) & 1;
			nBuff = nBuffs[nStep];
		}
	}
	return *resultBuffs[rStep];
}

BigInteger& BigInteger::Mod(const BigInteger& num)
{
	if (num > *this)
		return *this;
	bool isPowerOfTwo = true;
	for (uint32 i = 0; i < num.parts_.GetLen() - 1; i++)
	{
		if (!(isPowerOfTwo = num.parts_[i] == 0))
			break;
	}
	if (isPowerOfTwo && IsPowerOfTwo(num.parts_.Last()))
	{
		CutHeadBits(*this, num.GetBitsCount());
		return *this;
	}
	BigInteger aligned = num;
	ModAlign(aligned, *this);
	uint32 modCount = 0;
	uint32 subCount = 0;
	do
	{
		if (*this < aligned)
		{
			aligned = num;
			ModAlign(aligned, *this);
			++modCount;
		}
		*this -= aligned;
		++subCount;
	} while (*this >= num);
	return *this;
}

BigInteger& BigInteger::ShiftLeft()
{
	uint32 carry = 0;
	for (uint32 i = 0; i < parts_.GetLen(); ++i)
	{
		uint32 nextCarry = (uint32)((parts_[i] & 0x80000000) > 0);
		parts_[i] = (parts_[i] << 1) | carry;
		carry = nextCarry;
	}
	if (carry)
		parts_.Add(1);
	return *this;
}

BigInteger& BigInteger::ShiftRight()
{
	uint32 carry = 0;
	uint32 count = parts_.GetLen();
	uint32* data = parts_.GetData();
	for (int32 i = count - 1; i >= 0; --i)
	{
		uint32 newCarry = (data[i] & 1) << 31;
		data[i] = data[i] >> 1 | carry;
		carry = newCarry;
	}
	BigInteger_Trim(parts_);
	return *this;
}

BigInteger& BigInteger::ShiftLeft(uint32 bitCount)
{
	uint32 addedParts = bitCount / 32;
	uint32 partOffset = bitCount - addedParts * 32;
	// смещение кратно размеру составной единицы (разряду) числа
	if (partOffset == 0 && !addedParts) return *this;
	parts_.SetCapacity(addedParts + parts_.GetLen() + (partOffset ? 1 : 0));
	parts_.PrependFill(addedParts, 0);
	if (partOffset)
	{
		uint32 carry = 0;
		uint32 mask = 1;
		uint32 maskOffset = 32 - partOffset;
		for (uint32 i = 1; i < partOffset; ++i)
		{
			mask = mask << 1 | 1;
		}
		mask <<= maskOffset;
		for (uint32 i = addedParts; i < parts_.GetLen(); ++i)
		{
			uint32 nextCarry = (parts_[i] & mask) >> maskOffset;
			parts_[i] = parts_[i] << partOffset | carry;
			carry = nextCarry;
		}
		if (carry)
		{
			parts_.Add(carry);
		}
	}
	return *this;
}

BigInteger& BigInteger::ShiftRight(uint32 bitCount)
{
	uint32 removedParts = bitCount / 32;
	uint32 partOffset = bitCount - removedParts * 32;
	if (partOffset == 0 && !removedParts) return *this;
	if (removedParts >= parts_.GetLen())
	{
		parts_.Clear();
		parts_.Add(0);
		return *this;
	}
	else
		parts_.RemoveAt(0, removedParts);
	if (partOffset)
	{
		uint32 mask = 1;
		uint32 maskOffset = 32 - partOffset;
		for (uint32 i = 1; i < partOffset; ++i)
		{
			mask = mask << 1 | 1;
		}
		uint32 carry = 0;
		uint32 count = parts_.GetLen();
		uint32* data = parts_.GetData();
		for (int32 i = count - 1; i >= 0; --i)
		{
			uint32 carryNext = (data[i] & mask) << maskOffset;
			data[i] = data[i] >> partOffset | carry;
			carry = carryNext;
		}
		BigInteger_Trim(parts_);
	}
	return *this;
}

BigInteger& BigInteger::operator=(const BigInteger& n)
{
	if (*this == n)
		return *this;
	uint32 len = n.parts_.GetLen();
	parts_.Clear();
	parts_.ZeroFill(len);
	memcpy(parts_.GetBuffer(), n.parts_.GetData(), sizeof(uint32) * len);
	return *this;
}

int32 BigInteger::Compare(const BigInteger& num1, const BigInteger& num2)
{
	if (num1.parts_.GetLen() > num2.parts_.GetLen())
		return 1;
	if (num1.parts_.GetLen() < num2.parts_.GetLen())
		return -1;
	for (int32 i = num1.parts_.GetLen() - 1; i >= 0; --i)
	{
		if (num1.parts_[i] == num2.parts_[i])
			continue;
		if (num1.parts_[i] > num2.parts_[i])
			return 1;
		if (num1.parts_[i] < num2.parts_[i])
			return -1;
	}
	return 0;
}

void BigInteger::ModAlign(BigInteger& alignable, const BigInteger& ref)
{
	if (alignable == ref)
		return;
	assert(alignable.parts_.GetLen() < 2 || alignable.parts_.Last() != 0);
	assert(ref.parts_.GetLen() < 2 || ref.parts_.Last() != 0);
	assert(alignable < ref);
	int32 aBit = MostSignedBit(alignable.parts_.Last());
	int32 rBit = MostSignedBit(ref.parts_.Last());
	aBit = aBit == 0 ? 1 : aBit;
	rBit = rBit == 0 ? 1 : rBit;
	int32 bitOffset = rBit - aBit;
	uint32 offset = (ref.parts_.GetLen() - alignable.parts_.GetLen()) * 32 + bitOffset;
	alignable.ShiftLeft(offset);
	if (!offset || alignable < ref)
		return;
	alignable.ShiftRight();
}

uint32 BigInteger::DivAlign(BigInteger& alignable, const BigInteger& ref, BigInteger& mult)
{
	if (alignable == ref)
		return 0;
	assert(alignable.parts_.GetLen() < 2 || alignable.parts_.Last() != 0);
	assert(ref.parts_.GetLen() < 2 || ref.parts_.Last() != 0);
	assert(alignable < ref);
	int32 aBit = MostSignedBit(alignable.parts_.Last());
	int32 rBit = MostSignedBit(ref.parts_.Last());
	aBit = aBit == 0 ? 1 : aBit;
	rBit = rBit == 0 ? 1 : rBit;
	int32 bitOffset = rBit - aBit;
	uint32 offset = (ref.parts_.GetLen() - alignable.parts_.GetLen()) * 32 + bitOffset;
	mult = 1;
	mult.ShiftLeft(offset);
	alignable.ShiftLeft(offset);
	if (!offset || alignable < ref)
		return offset;
	alignable.ShiftRight();
	mult.ShiftRight();
	return --offset;
}

void BigInteger::CutHeadBits(BigInteger& num, uint32 bits)
{
	uint32 skipParts = bits / 32;
	uint32 skipBits = bits - skipParts * 32;
	uint32 partsRequirement = skipParts + (skipBits ? 1 : 0);
	if (num.parts_.GetLen() <= skipParts)
		return;
	uint32 mask = (1 << skipBits) - 1;
	if (!(num.parts_[skipParts] &= mask))
	{
		num.parts_.RemoveLast();
	}
	int32 remCount = num.parts_.GetLen() - partsRequirement;
	num.parts_.RemoveLast(remCount);
	BigInteger_Trim(num.parts_);
}

char* BigInteger::ToString()
{
	BigInteger_Trim(parts_);
	if (IsZero())
	{
		char* result = GetCurrentGameSate()->tempMemory->Allocate<char>(2);
		result[0] = '0';
		result[1] = '\0';
		return result;
	}
	const BigInteger ten(10);
	uint32* data = parts_.GetData();
	uint32 len = parts_.GetLen();
	uint32 millsCount = parts_.GetLen() * 10 / 9 + 2;
	uint32* mills = GetCurrentGameSate()->tempMemory->Allocate<uint32>(millsCount);
	memset(mills, 0, sizeof(uint32) * millsCount);

	uint32 offset = 0;
	int32 pos = len;
	while (--pos >= 0)
	{
		uint32 lo = data[pos];
		for (uint32 i = 0; i < offset; ++i)
		{
			uint64 merged = ((uint64)mills[i] << 32) | lo;
			mills[i] = (uint32)(merged % 1000000000);
			lo = (uint32)(merged / 1000000000);
		}
		if (lo)
		{
			mills[offset++] = lo % 1000000000;
			lo /= 1000000000;
			if (lo)
				mills[offset++] = lo;
		}
	}
	assert((uint64)offset * 9 < 0x100000000);
	uint32 sz = offset * 9;
	uint32 outDataPos = sz;
	char* outData = GetCurrentGameSate()->tempMemory->Allocate<char>(sz + 1);
	memset(outData, 0, sz + 1);
	uint32 mill = 0;
	for (uint32 i = 0; i < offset - 1; ++i)
	{
		mill = mills[i];
		int32 millDigits = 9;
		while (--millDigits >= 0)
		{
			outData[--outDataPos] = (char)(0x30 + mill % 10);
			mill /= 10;
		}
	}
	mill = mills[offset - 1];
	while (mill)
	{
		outData[--outDataPos] = (char)(0x30 + mill % 10);
		mill /= 10;
	}
	if (outDataPos)
	{
		memcpy(outData, outData + outDataPos, sz - outDataPos + 1);
	}
	return outData;
}

uint32 BigInteger::ToByteArray(byte* data, uint32 len)
{
	uint32 byteSize = parts_.GetLen() * sizeof(uint32);
	if (byteSize > len)
		return parts_.GetLen() * 4;
	uint32 numLen = parts_.GetLen() - 1;
	uint32* numData = parts_.GetData();
	byte* result = data;
	for (uint32 i = 0; i < numLen; ++i)
	{
		result[i * sizeof(uint32) + 0] = (byte)numData[i];
		result[i * sizeof(uint32) + 1] = (byte)(numData[i] >> 8);
		result[i * sizeof(uint32) + 2] = (byte)(numData[i] >> 16);
		result[i * sizeof(uint32) + 3] = (byte)(numData[i] >> 24);
	}
	byte b0, b1, b2, b3;
	uint32 count = 0;
	if (b3 = (byte)(numData[numLen] >> 24))
	{
		result[numLen * sizeof(uint32) + 3] = b3;
		++count;
	}
	if (b2 = (byte)(numData[numLen] >> 16))
	{
		result[numLen * sizeof(uint32) + 2] = b2;
		if (!count)
			byteSize -= 1;
		++count;
	}
	if (b1 = (byte)(numData[numLen] >> 8))
	{
		result[numLen * sizeof(uint32) + 1] = b1;
		if (!count)
			byteSize -= 2;
		++count;
	}
	if (b0 = (byte)numData[numLen])
	{
		result[numLen * sizeof(uint32) + 0] = b0;
		if (!count)
			byteSize -= 3;
		++count;
	}
	else if (!count)
		byteSize -= 4;
	return byteSize;
}

MemoryBlock BigInteger::ToByteArray()
{
	uint32 byteSize = parts_.GetLen() * sizeof(uint32);
	byte* result = GetCurrentGameSate()->tempMemory->Allocate<byte>(byteSize);
	return MemoryBlock{ result, ToByteArray(result, byteSize) };
}

BigInteger& operator+=(BigInteger& num1, const BigInteger& num2)
{
	uint32 carry = 0;
	uint64 accum;
	uint32 len1 = num1.parts_.GetLen();
	uint32 len2 = num2.parts_.GetLen();
	uint32* data1 = num1.parts_.GetData();
	uint32* data2 = num2.parts_.GetData();
	uint32 sharedLen = len2 > len1 ? len1 : len2;

	for (uint32 i = 0; i < sharedLen; ++i)
	{
		accum = (uint64)data1[i] + data2[i] + carry;
		carry = (uint32)(accum >> 32);
		data1[i] = (uint32)accum;
	}

	for (uint32 i = sharedLen; i < len2; ++i)
	{
		accum = (uint64)data2[i] + carry;
		carry = (uint32)(accum >> 32);
		num1.parts_.Add((uint32)accum);
	}
	uint32 offset = len2;
	while (carry)
	{
		if (offset < len1)
		{
			accum = (uint64)data1[offset] + carry;
			data1[offset++] = (uint32)accum;
			carry = (uint32)(accum >> 32);
		}
		else
		{
			num1.parts_.Add(carry);
			break;
		}
	}
	return num1;
}

BigInteger& operator*=(BigInteger& num1, const BigInteger& num2)
{
	List<uint32> result;
	BigInteger_MultiplePrecisionMulOffset(num1.parts_, num2.parts_, result, 0);
	num1.parts_ = (List<uint32>&&)result;
	return num1;
}

BigInteger& operator-=(BigInteger& num1, const BigInteger& num2)
{
	assert(num1 >= num2);
	uint32 carry = 0;
	uint32 nextCarry = 0;
	uint32* data1 = num1.parts_.GetData();
	uint32* data2 = num2.parts_.GetData();
	uint32 len1 = num1.parts_.GetLen();
	uint32 len2 = num2.parts_.GetLen();
	for (uint32 i = 0; i < len2; ++i)
	{
		nextCarry = data1[i] < carry || data1[i] - carry < data2[i];
		uint64 accum = nextCarry ?
			(uint64)data1[i] + 0x0100000000 - carry - num2.parts_[i] :
			(uint64)data1[i] - carry - data2[i];
		data1[i] = (uint32)accum;
		carry = nextCarry;
	}
	uint32 offset = len2;
	while (carry)
	{
		nextCarry = data1[offset] < carry;
		data1[offset] -= carry;
		carry = nextCarry;
		++offset;
	}
	BigInteger_Trim(num1.parts_);
	return num1;
}

BigInteger operator/(const BigInteger& num1, const BigInteger& num2)
{
	if (num1 < num2)
		return BigInteger(0);
	bool isPowerOfTwo = true;
	for (uint32 i = 0; i < num2.parts_.GetLen() - 1; i++)
	{
		if (!(isPowerOfTwo = num2.parts_[i] == 0))
			break;
	}
	if (isPowerOfTwo && IsPowerOfTwo(num2.parts_.Last()))
	{
		return BigInteger(num1).ShiftRight(num2.GetBitsCount() - 1);
	}
	BigInteger alignable(num2);
	BigInteger mult(0);
	BigInteger dividend(num1);
	uint32 alignableOffset = BigInteger::DivAlign(alignable, num1, mult);
	uint32 subs = 0;
	BigInteger result(0);
	BigInteger multAccum(0);
	multAccum.parts_.SetCapacity(mult.parts_.GetLen() + 1);
	while (dividend >= num2)
	{
		dividend -= alignable;
		++subs;
		if (dividend < alignable)
		{
			BigInteger_MultiplePrecisionMul(mult.parts_, subs, multAccum.parts_);
			BigInteger_Sum(result.parts_, multAccum.parts_);
			subs = 0;
			uint32 offset = alignable.GetBitsCount() - dividend.GetBitsCount();
			if (offset > alignableOffset)
				break;
			alignableOffset -= offset;
			alignable.ShiftRight(offset);
			mult.ShiftRight(offset);
			if (dividend < alignable)
			{
				if (alignableOffset == 0)
					break;
				--alignableOffset;
				alignable.ShiftRight();
				mult.ShiftRight();
			}
		}
	}

	return result;
}

BigInteger& operator/=(BigInteger& num1, const BigInteger& num2)
{
	num1.parts_ = (List<uint32>&&)(num1 / num2).parts_;
	return num1;
}