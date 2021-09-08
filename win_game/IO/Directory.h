#pragma once
#include "Path.h"
#include "Types/String.h"

namespace Directory
{
	// ����� ����������
	//
	// ���������:
	//		pattern - ������ ������������ �����, ������� ������� * � ?
	//
	// ����������:
	//		������ �������� �����
	List<Path> ListFiles(const String& root, const String& pattern);
};
