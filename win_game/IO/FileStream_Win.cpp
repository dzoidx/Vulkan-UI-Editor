#include "FileStream.h"
#include "Logging.h"

bool FileStream::Closed() const
{
	return wfile_ == INVALID_HANDLE_VALUE;
}

bool FileStream::Close()
{
	if (wfile_ != INVALID_HANDLE_VALUE)
	{
		Flush();
		CloseHandle(wfile_);
		wfile_ = INVALID_HANDLE_VALUE;
		FileStreamFlag::SetInvalid(Flags_);
		return true;
	}
	return false;
}

bool FileStream::Open()
{
	Close();

	Position_ = 0;
	Length_ = 0;
	ReadBuffer_.Pos = 0;
	WriteBuffer_.Pos = 0;

	CREATEFILE2_EXTENDED_PARAMETERS extendedParams;
	extendedParams.dwSize = sizeof(extendedParams);
	extendedParams.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
	extendedParams.dwFileFlags = FILE_FLAG_SEQUENTIAL_SCAN;
	extendedParams.dwSecurityQosFlags = SECURITY_ANONYMOUS;
	extendedParams.lpSecurityAttributes = nullptr;
	extendedParams.hTemplateFile = nullptr;

	DWORD access;
	DWORD shareMode;
	DWORD disposition;
	if (FileStreamFlag::TestRead(Flags_))
	{
		access = GENERIC_READ;
		shareMode = FILE_SHARE_READ;
		disposition = OPEN_EXISTING;
	}
	else
	{
		access = GENERIC_WRITE;
		shareMode = FILE_SHARE_READ;
		disposition = OPEN_ALWAYS;
	}

	wfile_ = CreateFile2(FilePath_.ToWchar(), access, shareMode, disposition, &extendedParams);
	if (wfile_ == INVALID_HANDLE_VALUE)
	{
		FileStreamFlag::SetInvalid(Flags_);
		WinError("CreateFile2");
		return false;
	}

	FILE_STANDARD_INFO fileInfo = {};
	if (!GetFileInformationByHandleEx(wfile_, FileStandardInfo, &fileInfo, sizeof(fileInfo)))
	{
		FileStreamFlag::SetInvalid(Flags_);
		WinError("GetFileInformationByHandleEx");
		return false;
	}

	Length_ = fileInfo.EndOfFile.QuadPart;
	FileStreamFlag::UnsetInvalid(Flags_);
	return true;
}

uint32 FileStream::Read(byte* buffer, uint32 size)
{
	if (!CanRead())
		return 0;
	//TODO: buffered read
	DWORD read;
	if (ReadFile(wfile_, buffer, size, &read, nullptr) == 0)
	{
		return 0;
	}
	Position_ += read;
	return read;
}

uint32 FileStream::Write(const byte* buffer, uint32 size)
{
	if (!CanWrite())
		return 0;

	if (!WriteBuffer_.Valid())
	{
		DWORD written;
		if (!WriteFile(wfile_, buffer, size, &written, nullptr))
		{
			return 0;
		}
		Position_ += written;
		return written;
	}

	int32 bufferOffset = 0;
	while (size)
	{
		int64 freeSpace = WriteBuffer_.Size - WriteBuffer_.Pos;
		if (!freeSpace)
		{
			Flush();
		}
		else
		{
			void* writePos = WriteBuffer_.Data + WriteBuffer_.Pos;
			int32 writeCount = freeSpace < size ? (int32)freeSpace : size;
			memcpy(writePos, buffer + bufferOffset, writeCount);
			bufferOffset += writeCount;
			WriteBuffer_.Pos += writeCount;
			size -= writeCount;
			Position_ += writeCount;
		}
	}
	return size;
}

void FileStream::Flush()
{
	if (!WriteBuffer_.Valid() && !CanWrite())
		return;

	DWORD written;
	if (!WriteFile(wfile_, WriteBuffer_.Data, (DWORD)WriteBuffer_.Pos, &written, nullptr))
	{
		WinError("WriteFile");
		return;
	}
	WriteBuffer_.Pos -= written;
	Position_ += written;
}