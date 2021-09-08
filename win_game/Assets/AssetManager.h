#pragma once
#include "Asset.h"
#include "TextureAsset.h"
#include "IO/Path.h"
#include "Memory/MemoryManager.h"
#include "Types/HashTable.h"
#include "Types/String.h"

class AssetManager
{
public:
	static AssetManager* GetInstance();
	Asset Load(const String& resourceName);
	TextureAsset GenerateTexture(bool alpha, uint32 width, uint32 height, byte* data);
	TextureAsset LoadTexture(const String& resourceName);

	static const uint32 DefaultBufferSize = 1024;
private:
	AssetManager();

	HashTableT<uint32, Asset, SimpleHash<uint32>> assets_;
	Path path_;

	static const String PathSep;
	static volatile uint32 IdGenerator;
	friend  MemoryManager;
};
