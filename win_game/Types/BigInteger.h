#pragma once
#include "List.h"
#include "Types.h"
#include "Memory/MemoryManager.h"
#include "Utils/Numeric.h"

class BigInteger
{
public:
	BigInteger(uint32 num);
	BigInteger(const BigInteger& num);
	// с младшего разряда к старшему
	BigInteger(const uint32* data, uint32 len);
	BigInteger(const byte* data, uint32 len);

	static BigInteger Pow(const BigInteger& num, const BigInteger& pow);
	static BigInteger Square(const BigInteger& num);
	static BigInteger PowMod(const BigInteger& num, const BigInteger& pow, const BigInteger& mod);
	BigInteger& ShiftLeft();
	BigInteger& ShiftRight();
	BigInteger& ShiftLeft(uint32 bitCount);
	BigInteger& ShiftRight(uint32 bitCount);
	BigInteger& Mod(const BigInteger& num);
	uint32 GetBitsCount() const { return parts_.GetLen() > 1 ? (parts_.GetLen() - 1) * 32 + MostSignedBit(parts_.Last()) : MostSignedBit(parts_.Last()); }
	bool IsZero() { return !parts_.Last(); }
	char* ToString();
	uint32 Size() const { return parts_.GetLen() * sizeof(uint32); }
	uint32 ToByteArray(byte* data, uint32 len);
	// аллоцирует во временном хипе
	MemoryBlock ToByteArray();
	BigInteger& operator=(const BigInteger& n);
	BigInteger& operator=(uint32 num) { parts_.Clear(); parts_.Add(num); return *this; }
	friend bool operator>(const BigInteger& num1, const BigInteger& num2) { return Compare(num1, num2) == 1; }
	friend bool operator<(const BigInteger& num1, const BigInteger& num2) { return Compare(num1, num2) == -1; }
	friend bool operator>=(const BigInteger& num1, const BigInteger& num2) { return Compare(num1, num2) > -1; }
	friend bool operator<=(const BigInteger& num1, const BigInteger& num2) { return Compare(num1, num2) < 1; }
	friend bool operator==(const BigInteger& num1, const BigInteger& num2) { return Compare(num1, num2) == 0; }
	friend bool operator!=(const BigInteger& num1, const BigInteger& num2) { return Compare(num1, num2) != 0; }
	static int32 Compare(const BigInteger& num1, const BigInteger& num2);
	friend BigInteger operator+(const BigInteger& num1, const BigInteger& num2) { BigInteger result(num1); return result += num2; }
	friend BigInteger& operator+=(BigInteger& num1, const BigInteger& num2);
	friend BigInteger operator*(const BigInteger& num1, const BigInteger& num2) { BigInteger result(num1); return result *= num2; }
	friend BigInteger& operator*=(BigInteger& num1, const BigInteger& num2);
	friend BigInteger& operator-=(BigInteger& num1, const BigInteger& num2);
	friend BigInteger operator-(const BigInteger& num1, const BigInteger& num2) { BigInteger result(num1); return result -= num2; }
	friend BigInteger operator/(const BigInteger& num1, const BigInteger& num2);
	friend BigInteger& operator/=(BigInteger& num1, const BigInteger& num2);
private:
	BigInteger(const List<uint32>& parts);
	BigInteger(List<uint32>&& parts);
	static void ModAlign(BigInteger& alignable, const BigInteger& ref);
	static uint32 DivAlign(BigInteger& alignable, const BigInteger& ref, BigInteger& mult);
	static void CutHeadBits(BigInteger& num, uint32 bits);
	static BigInteger PowModSimple(const BigInteger& num, const BigInteger& pow, const BigInteger& mod);
private:
	uint32 flags_;
	List<uint32> parts_;
};
