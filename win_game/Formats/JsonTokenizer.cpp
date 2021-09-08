#include "JsonTokenizer.h"

#include <cstdlib>
#include <cstring>

JsonTokenizer::JsonTokenizer(String source)
	:source_(source)
{
	position_ = 0;
	line_ = 0;
	lastToken_ = JsonTokenType::None;
	name_ = L"";
}

JsonTokenType JsonTokenizer::NextToken()
{
	const char space_chars[] = " \t\r\n";
	const char bool_literal_chars[] = "tf";
	const int32 name_buffer_step = 20;
	const int32 value_buffer_step = 100;

	if (lastToken_ == JsonTokenType::End)
		return JsonTokenType::End;
	if (lastToken_ == JsonTokenType::Error)
		return JsonTokenType::Error;
	uint32 sourceLen = source_.GetLength();
	if (contextStack_.GetCount() == 0 && lastToken_ == JsonTokenType::ObjectEnd)
	{
		lastToken_ = JsonTokenType::End;
		return lastToken_;
	}
	const char32* data = &source_[position_];
	JsonTokenType nextType = JsonTokenType::None;
	char valueBuff[512];
	static int32 buffPos;
	int32 ecount;
	do
	{
		if (position_ >= sourceLen) {
			if (contextStack_.GetCount() == 0)
				nextType = JsonTokenType::End;
			else
				nextType = JsonTokenType::Error;
			break;
		}
		char32 ch = *data++;
		JsonTokenizerContext currentContext;
		if (ch == '\n') ++line_;
		switch (lastToken_)
		{
		case JsonTokenType::None:
			if (ch == '{') {
				contextStack_.Push(JsonTokenizerContext::Object);
				nextType = JsonTokenType::ObjectStart;
			}
			else if (ch == '[') {
				contextStack_.Push(JsonTokenizerContext::Array);
				nextType = JsonTokenType::ArrayStart;
			}
			else if (strchr(space_chars, ch) == nullptr)
				nextType = JsonTokenType::Error;
			break;
		case JsonTokenType::ObjectNextElement:
		case JsonTokenType::ObjectStart:
			if (ch == '"')
			{
				nextType = JsonTokenType::ElementNameStart;
				name_ = L"";
			}
			else if (strchr(space_chars, ch) == nullptr)
				nextType = JsonTokenType::Error;
			break;
		case JsonTokenType::ElementNameStart:
			if (ch == '\\')
			{
				ch = *data++;
				++position_;
				int32 dataLeft = sourceLen - position_;
				if (dataLeft > 1)
				{
					if (ch == 'n')
						name_ += (char32)'\n';
					else if (ch == 'b')
						name_ += (char32)'\b';
					else if (ch == 'f')
						name_ += (char32)'\f';
					else if (ch == 'r')
						name_ += (char32)'\r';
					else if (ch == 't')
						name_ += (char32)'\t';
					else if (ch == '\\')
						name_ += (char32)'\\';
					else if (ch == '"')
						name_ += (char32)'"';
					else if (ch == 'u')
					{
						if (dataLeft < 5)
						{
							// bad format
							position_ += dataLeft;
						}
						else
						{
							char32 utf32Char = 0;
							int32 utf8BuffSz = 3;
							for (int32 i = 0; i < 4; i++)
							{
								ch = *data++;
								++position_;
								if ((ch < '0' || ch > '9') &&
									(ch < 'a' || ch > 'f') &&
									(ch < 'A' || ch > 'F'))
								{
									nextType = JsonTokenType::Error;
									break;
								}
								else
								{
									char digit = 0;
									if (ch >= '0' && ch <= '9')
										digit = ch - '0';
									else if (ch >= 'a' && ch <= 'f')
										digit = ch - 'a' + 10;
									else if (ch >= 'A' && ch <= 'F')
										digit = ch - 'A' + 10;
									else
										assert(false);

									utf32Char <<= 4;
									utf32Char |= digit;
								}
							}
							if (nextType == JsonTokenType::None)
							{
								name_ += utf32Char;
							}
						}
					}
				}
			}
			else if (ch == '"')
			{
				nextType = JsonTokenType::ElementNameEnd;
			}
			else
			{
				name_ += (char32)ch;
			}
			break;
		case JsonTokenType::ElementNameEnd:
			if (ch == ':')
			{
				nextType = JsonTokenType::ElementValueStart;
			}
			else if (strchr(space_chars, ch) == nullptr)
				nextType = JsonTokenType::Error;
			break;
		case JsonTokenType::ArrayNextElement:
		case JsonTokenType::ArrayStart:
		case JsonTokenType::ElementValueStart:
			name_ = L"";
			buffPos = 0;
			if (ch >= '0' && ch <= '9')
			{
				nextType = JsonTokenType::NumberValueStart;
				ecount = 0;
				buffPos = 0;
				memset(valueBuff, 1, 512);
				--position_;
			}
			else if (ch == '"')
			{
				nextType = JsonTokenType::StringValueStart;
			}
			else if (strchr(bool_literal_chars, ch) != nullptr)
			{
				if (ch == 't')
					nextType = JsonTokenType::BooleanTrueValueStart;
				else
					nextType = JsonTokenType::BooleanFalseValueStart;
				buffPos = 1;
			}
			else if (ch == 'n')
			{
				nextType = JsonTokenType::NullValueStart;
				buffPos = 1;
			}
			else if (ch == '[')
			{
				contextStack_.Push(JsonTokenizerContext::Array);
				nextType = JsonTokenType::ArrayStart;
			}
			else if (ch == '{')
			{
				contextStack_.Push(JsonTokenizerContext::Object);
				nextType = JsonTokenType::ObjectStart;
			}
			else if (strchr(space_chars, ch) == nullptr)
				nextType = JsonTokenType::Error;
			break;
		case JsonTokenType::NumberValueStart:
			currentContext = contextStack_.Peek();
			if (ch == '.')
			{
				nextType = JsonTokenType::RealValueStart;
				valueBuff[buffPos++] = ch;
			}
			else if ((ch >= '0' && ch <= '9') || (buffPos == 0 && ch == '-'))
			{
				valueBuff[buffPos++] = ch;
			}
			else if (strchr(space_chars, ch) != nullptr || ch == ',' || (currentContext == JsonTokenizerContext::Object && ch == '}') || (currentContext == JsonTokenizerContext::Array && ch == ']'))
			{
				--position_;
				nextType = JsonTokenType::IntegerValueEnd;
				valueBuff[buffPos++] = 0;
				intVal_ = atoll(valueBuff);
			}
			else
			{
				nextType = JsonTokenType::Error;
			}
			break;
		case JsonTokenType::RealValueStart:
			currentContext = contextStack_.Peek();
			if (ch >= '0' && ch <= '9')
			{
				valueBuff[buffPos++] = ch;
			}
			else if (ecount == 0 && (ch == 'E' || ch == 'e'))
			{
				++ecount;
				valueBuff[buffPos++] = ch;
			}
			else if (strchr(space_chars, ch) != nullptr || ch == ',' || (currentContext == JsonTokenizerContext::Object && ch == '}') || (currentContext == JsonTokenizerContext::Array && ch == ']'))
			{
				--position_;
				nextType = JsonTokenType::RealValueEnd;
				valueBuff[buffPos++] = 0;
				realVal_ = atof(valueBuff);
			}
			else
			{
				nextType = JsonTokenType::Error;
			}
			break;
		case JsonTokenType::BooleanFalseValueStart:
			currentContext = contextStack_.Peek();
			if (strchr(space_chars, ch) != nullptr || ch == ',' || (currentContext == JsonTokenizerContext::Object && ch == '}') || (currentContext == JsonTokenizerContext::Array && ch == ']'))
			{
				if (buffPos != 5)
					nextType = JsonTokenType::Error;
				else
				{
					--position_;
					nextType = JsonTokenType::BooleanFalseValueEnd;
				}
			}
			else if (buffPos < 5 && "false"[buffPos] == ch)
			{
				++buffPos;
			}
			else
			{
				nextType = JsonTokenType::Error;
			}
			break;
		case JsonTokenType::BooleanTrueValueStart:
			currentContext = contextStack_.Peek();
			if (strchr(space_chars, ch) != nullptr || ch == ',' || (currentContext == JsonTokenizerContext::Object && ch == '}') || (currentContext == JsonTokenizerContext::Array && ch == ']'))
			{
				if (buffPos != 4)
					nextType = JsonTokenType::Error;
				else
				{
					--position_;
					nextType = JsonTokenType::BooleanTrueValueEnd;
				}
			}
			else if (buffPos < 4 && "true"[buffPos] == ch)
			{
				++buffPos;
			}
			else
			{
				nextType = JsonTokenType::Error;
			}
			break;
		case JsonTokenType::NullValueStart:
			currentContext = contextStack_.Peek();
			if (strchr(space_chars, ch) != nullptr || ch == ',' || (currentContext == JsonTokenizerContext::Object && ch == '}') || (currentContext == JsonTokenizerContext::Array && ch == ']'))
			{
				if (buffPos != 4)
					nextType = JsonTokenType::Error;
				else
				{
					--position_;
					nextType = JsonTokenType::NullValueEnd;
				}
			}
			else if (buffPos < 4 && "null"[buffPos] == ch)
			{
				++buffPos;
			}
			else
			{
				nextType = JsonTokenType::Error;
			}
			break;
		case JsonTokenType::StringValueStart:
			if (ch == '\\')
			{
				ch = *data++;
				++position_;
				int32 dataLeft = sourceLen - position_;
				if (dataLeft > 1)
				{
					if (ch == 'n')
						name_ += (char32)'\n';
					else if (ch == 'b')
						name_ += (char32)'\b';
					else if (ch == 'f')
						name_ += (char32)'\f';
					else if (ch == 'r')
						name_ += (char32)'\r';
					else if (ch == 't')
						name_ += (char32)'\t';
					else if (ch == '\\')
						name_ += (char32)'\\';
					else if (ch == '"')
						name_ += (char32)'"';
					else if (ch == 'u')
					{
						if (dataLeft < 5)
						{
							// bad format
							position_ += dataLeft;
						}
						else
						{
							char32 utf32Char = 0;
							int32 utf8BuffSz = 3;
							for (int32 i = 0; i < 4; i++)
							{
								ch = *data++;
								++position_;
								if ((ch < '0' || ch > '9') &&
									(ch < 'a' || ch > 'f') &&
									(ch < 'A' || ch > 'F'))
								{
									nextType = JsonTokenType::Error;
									break;
								}
								else
								{
									char digit = 0;
									if (ch >= '0' && ch <= '9')
										digit = ch - '0';
									else if (ch >= 'a' && ch <= 'f')
										digit = ch - 'a' + 10;
									else if (ch >= 'A' && ch <= 'F')
										digit = ch - 'A' + 10;
									else
										assert(false);

									utf32Char <<= 4;
									utf32Char |= digit;
								}
							}
							if (nextType == JsonTokenType::None)
							{
								name_ += utf32Char;
							}
						}
					}
				}
			}
			else if (ch == '"')
			{
				nextType = JsonTokenType::StringValueEnd;
			}
			else
			{
				name_ += (char32)ch;
			}
			break;
		case JsonTokenType::ObjectEnd:
		case JsonTokenType::ArrayEnd:
		case JsonTokenType::RealValueEnd:
		case JsonTokenType::StringValueEnd:
		case JsonTokenType::NullValueEnd:
		case JsonTokenType::IntegerValueEnd:
		case JsonTokenType::BooleanFalseValueEnd:
		case JsonTokenType::BooleanTrueValueEnd:
			if (strchr(space_chars, ch) != nullptr)
			{
				// skip all space chars
			}
			else
			{
				currentContext = contextStack_.Peek();
				if (ch == '}' && currentContext == JsonTokenizerContext::Object)
				{
					currentContext = contextStack_.Pop();
					nextType = JsonTokenType::ObjectEnd;
				}
				else if (ch == ']' && currentContext == JsonTokenizerContext::Array)
				{
					currentContext = contextStack_.Pop();
					if (currentContext == JsonTokenizerContext::None)
					{
						nextType = JsonTokenType::End;
					}
					else
					{
						nextType = JsonTokenType::ArrayEnd;
					}
				}
				else if (ch == ',')
				{
					if (currentContext == JsonTokenizerContext::Object)
						nextType = JsonTokenType::ObjectNextElement;
					else if (currentContext == JsonTokenizerContext::Array)
						nextType = JsonTokenType::ArrayNextElement;
					else
						nextType = JsonTokenType::Error;
				}
				else
					nextType = JsonTokenType::Error;
			}
			break;
		case JsonTokenType::Error:
		case JsonTokenType::End:
			break;

		}
		if (nextType != JsonTokenType::Error)
			++position_;
	} while (nextType == JsonTokenType::None);

	return lastToken_ = nextType;
}