#include "Color.h"

#include "Logging.h"
#include "Utils/StringBuilder.h"

Color::Color(String& str)
	:Color(0)
{
	// не можем кидать ассерты тут ради работы хотрелоада в случае кос€чного джсона
	if (str.GetLength() != 9 ||
		str[0] != '#')
	{
		logger.Error(String::Format("Bad color format: '{0}'.")
			.Set(0, str).ToUtf8()
		);
		return;
	}
	List<byte> bytes;
	int32 badCode = String::ReadBytesFromHex(str, 1, bytes);
	if (badCode > 0)
	{
		logger.Error(String::Format("Bad color format: '{0}' at {1}.")
			.Set(0, str)
			.Set(1, badCode)
			.ToUtf8()
		);
		return;
	}
	r = bytes[0] / 255.0f;
	g = bytes[1] / 255.0f;
	b = bytes[2] / 255.0f;
	a = bytes[3] / 255.0f;
}
