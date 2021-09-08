#pragma once
#include "String.h"

struct Color
{
	real32 r, g, b, a;

	Color() : Color(0) {}
	Color(real32 gray) : Color(gray, gray, gray, 1.0f) {}
	Color(real32 red, real32 green, real32 blue) : Color(red, green, blue, 1.0f) {}
	Color(real32 red, real32 green, real32 blue, real32 alpha) : r(red), g(green), b(blue), a(alpha) {}
	Color(String& str);
};
