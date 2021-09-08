#include "TextureAsset.h"

#include "AssetManager.h"
#include "Memory/Endianness.h"

TextureAsset AssetManager::GenerateTexture(bool alpha, uint32 width, uint32 height, byte* data)
{
	return TextureAsset(alpha, width, height, data);
}

TextureAsset AssetManager::LoadTexture(const String& resourceName)
{
	//TODO: TextureAsset cache
	Asset asset(Load(resourceName));
	if (asset.GetType() == AssetType::Texture)
		return TextureAsset((Asset&&)asset, ++IdGenerator);
	return TextureAsset();
}

TextureAsset::TextureAsset(Asset&& asset, uint32 id)
{
	Asset_ = (Asset&&)asset;
	Id = id;
}

TextureAsset::TextureAsset(bool alpha, uint32 w, uint32 h, byte* data)
{
	Width = w;
	Height = h;
	Data = data;
	Alpha = alpha;
	BytesPerPixel = alpha ? 4 : 3;
}

void TextureAsset::LoadSync()
{
	if (Asset_.GetType() != AssetType::Texture)
		return;
	if (Data != nullptr)
		return;

	Asset_.Load();
	byte* data = Asset_.GetData();

	int32 width = *((int32*)data);
	int32 height = *((int32*)data + 1);
	byte alpha = *(data + 8);
	Width = NTOHL(width);
	Height = NTOHL(height);
	Alpha = alpha == 1;
	Data = data + HeaderSize;
	BytesPerPixel = alpha ? 4 : 3;
}