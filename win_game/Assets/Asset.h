#pragma once
#include "Types.h"
#include "IO/FileStream.h"

enum class AssetType
{
	None,
	Error,
	Binary,
	// текстовые данные в формате UTF-8
	Text,
	Json,
	Texture,
	SpriteAtlas,
	Audio,
	DHKey,
	COUNT
};

class Asset
{
public:
	Asset() : len_(), data_(), type_(AssetType::Error), stream_(), id_() {}
	Asset(Asset&& a)
	{
		type_ = a.type_;
		len_ = a.len_;
		data_ = a.data_;
		stream_ = (FileStream&&)a.stream_;
		filePath_ = (String&&)a.filePath_;
		a.data_ = nullptr;
		a.len_ = 0;
		a.type_ = AssetType::Error;
	}
	Asset(const String& filePath, FileStream&& stream);
	Asset(uint32 id, const String& filePath, AssetType type);
	~Asset();
	AssetType GetType() const { return type_; }
	bool InMemory() const { return data_ != nullptr; }
	void Load();
	void Unload();
	byte* GetData() const { return data_; }

	Asset& operator=(Asset&& a)
	{
		type_ = a.type_;
		len_ = a.len_;
		data_ = a.data_;
		stream_ = (FileStream&&)a.stream_;
		a.data_ = nullptr;
		a.len_ = 0;
		a.type_ = AssetType::Error;
		return *this;
	}

protected:
	bool ReadAll();

	uint64 len_;
	byte* data_;
	String filePath_;
	FileStream stream_;
	AssetType type_;
	uint32 id_;

private:
	static const uint32 HeaderSize = 8;

	friend class AssetManager;
};