#include "AssetManager.h"
#include "Logging.h"
#include "Windows.h"
#include "Memory/Endianness.h"
#include "Types/GrowingBuffer.h"
#include "Utils/StringBuilder.h"

AssetManager::AssetManager()
{
	LPWSTR cmd = GetCommandLine();
	String args(cmd);
	List<String> argList = args.Split(' ');
	const Path appPath = Path(argList[0].Substring(1, argList[0].GetLength() - 2)).BasePath();
	Path assetLookup = appPath.Cd("Assets");
	if (assetLookup.IsDir())
	{
		path_ = (Path&&)assetLookup;
	}
	else
	{
		assetLookup = appPath.Cd("..").Cd("..").Cd("..").Cd("Assets");
		assert(assetLookup.IsDir());
		path_ = (Path&&)assetLookup;
	}

	Path assetRoot(path_);
	assetRoot.Cd("asset.list");

	assert(!assetRoot.IsFile());
	if (!assetRoot.IsFile())
	{
		Error(String::Format("Asset list not found.").ToUtf8());
		return;
	}

	// ид, тип, размер строки пути (3 инта)
	const uint32 assetHeaderSize = sizeof(uint32) * 3;
	FileStream assetList = FileStream::OpenRead(assetRoot.ToString());
	byte assetHeader[assetHeaderSize];
	GrowingBuffer strBuff(20);
	assetRoot.Cd("..");
	while (assetList.Read(assetHeader, assetHeaderSize))
	{
		uint32 id = NTOHL(*(uint32*)assetHeader);
		AssetType type = (AssetType)NTOHL(*(uint32*)&assetHeader[4]);
		uint32 strLen = NTOHL(*(uint32*)&assetHeader[8]);
		strBuff.EnsureSize(strLen, false);
		uint32 read = assetList.Read(strBuff.GetData(), strLen);
		assert(read == strLen);
		String path = String((char*)strBuff.GetData(), (int32)strLen);
		Path assetPath = Path(assetRoot).Cd(path);
		if (!assetPath.IsFile())
		{
			Error(String::Format("Asset load failed: {0}.").Set(0, assetPath.ToString()).ToUtf8());
			continue;
		}
		if (!assets_.Insert(id, (Asset&&)Asset(id, assetPath.ToString(), type), true))
		{
			Error(String::Format("Asset with id '{0}' already present.").Set(0, id).ToUtf8());
			continue;
		}
	}
}
