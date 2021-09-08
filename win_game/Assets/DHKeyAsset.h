#pragma once
#include "Asset.h"
#include "Types/BigInteger.h"

// generate new
// openssl dhparam 100 | openssl dhparam -text -noout
// there 100 is bit size of prime
// opensll outputs prime in big-endian byte order
class DHKeyAsset
{
public:
	DHKeyAsset() : asset_(), Prime(0), Root(0) {}
	DHKeyAsset(Asset&& asset, uint32 id) : asset_((Asset&&)asset), Id(id), Prime(0), Root(0) {}

	void Load();

	BigInteger Prime;
	BigInteger Root;

public:
	uint32 Id;
private:
	byte* data_;
	Asset asset_;
};
