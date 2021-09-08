#pragma once
#include "Types.h"
#include "UINode.h"
#include "Types/Color.h"

class UIImage : public UINode
{
public:
	UIImage()
		:UINode()
	{
		Type = UINodeType::Image;
		RenderData = nullptr;
		TextureSlot = 7;
	}
	~UIImage() override
	{
	    delete RenderData;
	}

	Color Color;
	int32 TextureSlot;

	void* RenderData;
};
