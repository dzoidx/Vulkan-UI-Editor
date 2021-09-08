#pragma once
#include "Types/String.h"

class Path
{
public:
	Path() {}
	Path(const String& path);
	Path Cd(const String& path) const { return Path(*this).Cd(path); }
	Path& Cd(const String& path);

	// если это путь к файлу, то удаляет имя файла из пути
	Path& BasePath();
	Path BasePath() const { return Path(*this).BasePath(); }
	bool Exists() const { return isValid_; }
	bool IsDir() const { return isValid_ && isDir_; }
	bool IsFile() const { return isValid_ && !isDir_; }
	const String& GetExt() const { return ext_; }

	String ToString() const { return String::Join(DirectorySep, parts_); }
private:
	bool Cd_(String&& path);
	void GetExt_();
private:
	bool isValid_;
	bool isDir_;
	String ext_;
	List<String> parts_;

public:
	static const String DirectorySep;
	static const String MoveUp;
	static const String Dot;
};
