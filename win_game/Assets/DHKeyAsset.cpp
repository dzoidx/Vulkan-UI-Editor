#include "DHKeyAsset.h"
#include "Memory/Endianness.h"

void DHKeyAsset::Load()
{
	if (asset_.GetType() != AssetType::DHKey)
		return;
	if (data_ != nullptr)
		return;

	asset_.Load();
	data_ = asset_.GetData();

	uint32 primeLen = *((uint32*)data_);
	uint32 rootLen = *((uint32*)data_ + 1);
	primeLen = NTOHL(primeLen);
	rootLen = NTOHL(rootLen);
	Prime = BigInteger(data_ + 8, primeLen);
	Root = BigInteger(data_ + 8 + primeLen, rootLen);
}
