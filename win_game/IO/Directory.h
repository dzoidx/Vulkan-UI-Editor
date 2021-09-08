#pragma once
#include "Path.h"
#include "Types/String.h"

namespace Directory
{
	// Обход директории
	//
	// Параметры:
	//		pattern - фильтр возвращаемых путей, включая символы * и ?
	//
	// Возвращает:
	//		Список валдиных путей
	List<Path> ListFiles(const String& root, const String& pattern);
};
