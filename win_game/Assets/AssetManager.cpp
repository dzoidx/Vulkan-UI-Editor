#include "AssetManager.h"

#include "Threading/MutexLock.h"

Asset AssetManager::Load(const String& resourceName)
{
	List<String> parts = resourceName.Split(PathSep);
	Path p(path_);
	Path* pp = &p;
	for (uint32 i = 0; i < parts.GetLen(); ++i)
	{
		pp = &pp->Cd((String&&)parts[i]);
	}
	if (!pp->IsFile())
		return Asset();

	String path = pp->ToString();
	FileStream stream = FileStream::OpenRead(path);
	AssetType type{};

	return Asset(path, (FileStream&&)stream);
}


Mutex assetManagerMutex;
AssetManager* assetManager;

const String AssetManager::PathSep = "/";
volatile uint32 AssetManager::IdGenerator = 0;

AssetManager* AssetManager::GetInstance()
{
	MutexLock lock;
	if (assetManager != nullptr)
		return assetManager;
	lock.Lock(&assetManagerMutex);
	if (assetManager != nullptr)
		return assetManager;
	assetManager = MemoryManager::Instance()->Allocate<AssetManager>();
	return assetManager;
}

#ifdef ANDROID
//TODO(android): platform asset manager
#endif

#ifdef IOS
//TODO(ios): platform asset manager
#endif