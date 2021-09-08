#include "FileStream.h"

FileStream::FileStream()
	:
	Flags_{},
#ifdef _WIN32
	wfile_(INVALID_HANDLE_VALUE),
#endif
	Position_(),
	Length_(),
	FilePath_(),
	ReadBuffer_(),
	WriteBuffer_()
{
	FileStreamFlag::SetInvalid(Flags_);
}

FileStream::FileStream(const String& fileName)
	:FileStream()
{
	FilePath_ = fileName;
}

FileStream::FileStream(FileStream&& f)
{
	Flags_ = f.Flags_;
#ifdef _WIN32
	wfile_ = f.wfile_;
#endif
	Position_ = f.Position_;
	Length_ = f.Length_;
	FilePath_ = (String&&)f.FilePath_;
	ReadBuffer_ = (CursorBuffer&&)f.ReadBuffer_;
	WriteBuffer_ = (CursorBuffer&&)f.WriteBuffer_;
	f.ReadBuffer_.Data = nullptr;
	f.WriteBuffer_.Data = nullptr;
	FileStreamFlag::SetInvalid(f.Flags_);
#ifdef _WIN32
	f.wfile_ = INVALID_HANDLE_VALUE;
#endif
}

FileStream::~FileStream()
{
	Close();
}

FileStream& FileStream::operator=(FileStream&& f)
{
	Close();
	//TODO: deinit
	new (this) FileStream((FileStream&&)f);
	return *this;
}

bool FileStream::CanRead() const
{
	return Valid() && !Closed() && FileStreamFlag::TestRead(Flags_);
}

bool FileStream::CanWrite() const
{
	return Valid() && !Closed() && FileStreamFlag::TestWrite(Flags_);
}

bool FileStream::Valid() const
{
	return !FileStreamFlag::TestInvalid(Flags_);
}

FileStream FileStream::OpenRead(const String& fileName)
{
	FileStream result(fileName);
	FileStreamFlag::SetRead(result.Flags_);
	result.Open();
	return result;
}

FileStream FileStream::OpenWrite(const String& fileName)
{
	FileStream result(fileName);
	FileStreamFlag::SetWrite(result.Flags_);
	result.Open();
	return result;
}