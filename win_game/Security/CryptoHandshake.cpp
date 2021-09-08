#include "CryptoHandshake.h"

CryptoHandshake::~CryptoHandshake()
{
	if (publicKey_)
		MemoryManager::Instance()->Free<byte>(publicKey_, PublicKeyLen);
	if (PrivateKey)
		MemoryManager::Instance()->Free<byte>(PrivateKey, PrivateKeyLen);
}

void CryptoHandshake::SetRemotePublicKey(byte* key, uint32 len)
{
	BigInteger remotePublicKey(key, len);
	BigInteger privateKey = BigInteger::PowMod(remotePublicKey, secret_, prime_);
	uint32 byteSize = privateKey.Size();
	PrivateKey = MemoryManager::Instance()->Allocate<byte>(byteSize);
	PrivateKeyLen = privateKey.ToByteArray(PrivateKey, byteSize);
}

void CryptoHandshake::MakePublicKey()
{
	byte secretBytes[1024];
	RandomBytes(secretBytes, sizeof(secretBytes));
	secret_ = BigInteger(secretBytes, sizeof(secretBytes));
	BigInteger publicKey = BigInteger::PowMod(primeRoot_, secret_, prime_);
	uint32 byteSize = publicKey.Size();
	publicKey_ = MemoryManager::Instance()->Allocate<byte>(byteSize);
	PublicKeyLen = publicKey.ToByteArray(publicKey_, byteSize);
}