#include "SpriteAtlasAsset.h"

#include "Memory/Endianness.h"

void SpriteAtlasAsset::Load()
{
	if (asset_.GetType() != AssetType::SpriteAtlas)
		return;
	asset_.Load();
	byte* data = asset_.GetData();
	uint32 textureId = *((uint32*)data);
	uint32 spritesCount = *((uint32*)data + 1);
	TextureId = textureId = NTOHL(textureId);
	spritesCount = NTOHL(spritesCount);
	Sprites = List<Sprite>(spritesCount);
	Sprite* sprite = (Sprite*)(data + 8);
	//TODO: data size check
	for (uint32 i = 0; i < spritesCount; ++i)
	{
		Sprites.Add(*sprite);
	}
}
