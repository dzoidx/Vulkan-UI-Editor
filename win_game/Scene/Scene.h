#pragma once
#include "UI/UIImage.h"

struct Scene
{
	String Name;
	List<UIImage*> Nodes;

	Scene& operator=(Scene&& r)
	{
		Name = (String&&)r.Name;
		Nodes = (List<UIImage*>&&)r.Nodes;
		return *this;
	}
};
