#include "Directory.h"
#include "Windows.h"
#include "Logging.h"

namespace Directory
{
	List<Path> ListFiles(const String& root, const String& pattern)
	{
		List<Path> result;
		List<Path> dirs;
		Path p(root);
		p.Cd(pattern);
		const wchar_t* patternW = p.ToString().ToWchar();

		WIN32_FIND_DATA find;
		HANDLE h = FindFirstFile(patternW, &find);
		if (h == INVALID_HANDLE_VALUE)
		{
			if (GetLastError() == ERROR_FILE_NOT_FOUND)
				return result;
			LogWinError("ListFiles", __LINE__);
			return result;
		}

		String fileName = String(find.cFileName);
		Path filePath = Path(root);
		filePath.Cd(fileName);
		if (fileName != Path::MoveUp && fileName != Path::Dot)
		{
			if (!filePath.IsDir())
				result.Add((Path&&)filePath);
			else
				dirs.Add((Path&&)filePath);
		}

		while (true)
		{
			if (!FindNextFile(h, &find))
			{
				if (GetLastError() == ERROR_NO_MORE_FILES)
					break;
				LogWinError("ListFiles", __LINE__);
				return result;
			}

			fileName = String(find.cFileName);
			Path filePath = Path(root);
			filePath.Cd(fileName);
			if (fileName != Path::MoveUp && fileName != Path::Dot)
			{
				if (!filePath.IsDir())
					result.Add((Path&&)filePath);
				else
					dirs.Add((Path&&)filePath);
			}
		}
		if (!FindClose(h))
			LogWinError("ListFiles", __LINE__);

		for (uint32 i = 0; i < dirs.GetLen(); ++i)
		{
			List<Path> subList = ListFiles(dirs[i].ToString(), pattern);
			result.Add(subList);
		}

		return result;
	}
}
