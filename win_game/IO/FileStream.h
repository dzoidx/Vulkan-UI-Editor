#pragma once
#ifdef _WIN32
#include "Windows.h"
#endif
#include "Types/String.h"

struct CursorBuffer
{
	uint32 Size;
	uint32 Pos;
	byte* Data;

	bool Valid() const { return Data != nullptr; }
};

namespace FileStreamFlag
{
	constexpr uint32 Read = 0x00000001;
	constexpr uint32 Write = 0x00000002;
	constexpr uint32 Invalid = 0x00000004;

	inline bool TestRead(uint32 flags) { return flags & Read; }
	inline bool TestWrite(uint32 flags) { return flags & Write; }
	inline bool TestInvalid(uint32 flags) { return flags & Invalid; }
	inline void SetRead(uint32& flags) { flags |= Read; }
	inline void SetWrite(uint32& flags) { flags |= Write; }
	inline void SetInvalid(uint32& flags) { flags |= Invalid; }
	inline void UnsetInvalid(uint32& flags) { flags &= ~Invalid; }
}


// TODO: разобраться с буфуризацией
struct FileStream
{
	uint32 Flags_;
#ifdef _WIN32
	HANDLE wfile_;
#endif
	uint64 Position_;
	uint64 Length_;
	String FilePath_;
	CursorBuffer ReadBuffer_;
	CursorBuffer WriteBuffer_;

	bool Valid() const;
	bool CanRead() const;
	bool CanWrite() const;
	bool Closed() const;
	bool Close();
	bool Open();

	FileStream();
	FileStream(const String& fileName);
	FileStream(FileStream&& f);
	~FileStream();
	FileStream& operator=(FileStream&& f);

	uint32 Read(byte* buffer, uint32 size);
	uint32 Write(const byte* buffer, uint32 size);
	void Flush();

	static FileStream OpenRead(const String& fileName);
	static FileStream OpenWrite(const String& fileName);
};