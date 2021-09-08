#include "Path.h"

#ifdef _WIN32
#include "Windows.h"
const String Path::DirectorySep = "\\";
#endif
#if POSIX
const String Path::DirectorySep = "/";
#endif
const String Path::MoveUp = L"..";
const String Path::Dot = L".";

bool IsDir(const String& s)
{
#if _WIN32
	DWORD dwAttrib = GetFileAttributes(s.ToWchar());
	return dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);

#elif
	struct stat buffer;
	int         status;
	status = stat(s.ToUtf8(), &buffer);
	return status == 0 && S_ISDIR(buffer.st_mode);
#endif
}

bool PathExists(const String& s)
{
#if _WIN32
	DWORD dwAttrib = GetFileAttributes(s.ToWchar());
	return dwAttrib != INVALID_FILE_ATTRIBUTES;

#elif
	struct stat buffer;
	int         status;
	status = stat(s.ToUtf8(), &buffer);
	return status == 0;
#endif
}

Path::Path(const String& path)
{
	parts_ = path.Split(DirectorySep);
	isValid_ = ::PathExists(path);
	isDir_ = isValid_ && ::IsDir(path);
	GetExt_();
}

Path& Path::Cd(const String& path)
{
	List<String> parts = path.Split(DirectorySep);
	for (uint32 i = 0; i < parts.GetLen(); ++i)
	{
		if (!Cd_((String&&)parts[i]))
			return *this;
	}
	return *this;
}

Path& Path::BasePath()
{
	if (!IsFile())
		return *this;
	if (parts_.GetLen() < 1)
		return *this;
	parts_.RemoveLast();
	String fullPath = ToString();
	isValid_ = ::PathExists(fullPath);
	isDir_ = isValid_ && ::IsDir(fullPath);
	GetExt_();
	return *this;
}

bool Path::Cd_(String&& path)
{
	if (path == Dot)
		return true;
	if (path == MoveUp)
	{
		if (parts_.GetLen() < 1)
			return false;
		parts_.RemoveLast();
		return true;
	}

	parts_.Add((String&&)path);
	String fullPath = ToString();
	isValid_ = ::PathExists(fullPath);
	isDir_ = isValid_ && ::IsDir(fullPath);
	GetExt_();
	return isValid_;
}

void Path::GetExt_()
{
	if (!isValid_ || isDir_)
		return;
	String& fileName = parts_.Last();
	int32 dotIdx = fileName.LastIndexOf(".", 0);
	if (dotIdx < 0)
	{
		ext_ = String();
	}
	else
	{
		ext_ = fileName.Substring(dotIdx + 1);
	}
}