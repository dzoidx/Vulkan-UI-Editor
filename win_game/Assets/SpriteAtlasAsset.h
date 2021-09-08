#pragma once
#include "Asset.h"
#include "Sprite.h"

struct SpriteAtlasAsset
{
	uint32 Id;
	uint32 TextureId;
	List<Sprite> Sprites;

	SpriteAtlasAsset(Asset&& asset, uint32 id) :asset_((Asset&&)asset), Id(id) {}

	void Load();
private:
	Asset asset_;
};
