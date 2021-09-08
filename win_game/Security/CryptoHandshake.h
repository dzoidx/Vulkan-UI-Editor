#pragma once
#include "Types.h"
#include "Assets/DHKeyAsset.h"
#include "Types/BigInteger.h"

class CryptoHandshake
{
public:
	CryptoHandshake(DHKeyAsset& asset) :prime_(asset.Prime), primeRoot_(asset.Root), secret_(0), publicKey_(0), PrivateKey(0), PublicKeyLen(0), PrivateKeyLen(0) {}
	~CryptoHandshake();

	byte* GetPublicKey() { if (!publicKey_) MakePublicKey(); return publicKey_; }
	// публичный ключ второй стороны. используется для вычислния общего секрета
	void SetRemotePublicKey(byte* key, uint32 len);
public:
	byte* PrivateKey;
	uint32 PrivateKeyLen;
	uint32 PublicKeyLen;
private:
	void MakePublicKey();
private:
	BigInteger prime_;
	BigInteger primeRoot_;
	BigInteger secret_;
	byte* publicKey_;
};
