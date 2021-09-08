#include "Label.h"

Label::Label()
{
}

Label::~Label()
{
}

void Label::SetText(const String& text)
{
	text_ = text;
	Dirty = true;
}

void Label::SetText(String&& text)
{
	text_ = text;
	Dirty = true;
}

void Label::SetFont(const Font& font)
{
	font_ = font;
	Dirty = true;
}

void Label::SetSize(int32 size)
{
	size_ = size;
	Dirty = true;
}