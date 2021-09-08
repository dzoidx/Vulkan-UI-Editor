#include "Asset.h"
#include "Logging.h"
#include "Memory/Endianness.h"
#include "Memory/MemoryManager.h"
#include "Threading/MutexLock.h"
#include "Utils/StringBuilder.h"


Asset::Asset(const String& filePath, FileStream&& stream)
	: data_(nullptr), stream_((FileStream&&)stream), filePath_(filePath)
{
	byte buff[HeaderSize];
	if (!stream_.CanRead())
		assert(false);
	assert(stream_.Length_ >= HeaderSize);
	uint32 read = stream_.Read(buff, HeaderSize);
	assert(read == HeaderSize);
	AssetType type = (AssetType)NTOHL(*(uint32*)buff);
	id_ = NTOHL(*((uint32*)buff + 1));
	type_ = type >= AssetType::COUNT ? AssetType::Error : type;
}

Asset::Asset(uint32 id, const String& filePath, AssetType type)
	:data_(nullptr), stream_(), filePath_(filePath), type_(type), id_(id)
{
}

Asset::~Asset()
{
	stream_.Close();
	Unload();
}

void Asset::Load()
{
	if (data_ != nullptr)
		return;
	ReadAll();
}

void Asset::Unload()
{
	if (data_ != nullptr)
	{
		MemoryManager::Instance()->FreeRaw(data_, (uint32)len_);
		data_ = nullptr;
		len_ = 0;
		stream_.Close();
	}
}

bool Asset::ReadAll()
{
	if (!stream_.Valid())
	{
		stream_ = FileStream::OpenRead(filePath_);
		if (!stream_.Valid())
		{
			Error(String::Format("Asset load failed. Can't read file '{0}'.").Set(0, filePath_).ToUtf8());
			return false;
		}
	}
	if (!stream_.CanRead())
	{
		Error(String::Format("Asset load failed. FileStream.CanRead = false. '{0}'.").Set(0, filePath_).ToUtf8());
		return false;
	}
	len_ = (uint64)stream_.Length_ - HeaderSize;
	assert(len_ < 0xFFFFFFFF);
	uint64 offset = 0;
	data_ = (byte*)MemoryManager::Instance()->AllocateRaw((uint32)len_);
	uint32 read;
	while (offset < len_ && (read = stream_.Read(&data_[offset], (uint32)len_)))
	{
		offset += read;
	}
	//TODO: вернуть курсор в начало, чтоб можно было еще раз прочитать файл например в случае Unload
	return true;
}
