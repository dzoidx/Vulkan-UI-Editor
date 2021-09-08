#pragma once
#include "UINode.h"
#include "Types/String.h"

class Font
{
public:
	Font() {};
	~Font() {};
private:

};

// Текстовое поле.
class Label : public UINode
{
public:
	Label();
	~Label();
public:
	void SetText(const String& text);
	void SetText(String&& text);
	void SetFont(const Font& font);
	void SetSize(int32 size);
private:
	int32 size_;
	String text_;
	Font font_;
};
