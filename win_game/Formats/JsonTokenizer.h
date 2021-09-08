#pragma once
#include "Types/Stack.h"
#include "Types/String.h"

enum class JsonTokenizerContext
{
	None,
	Object,
	Array
};

enum class JsonTokenType
{
	None,
	ObjectStart,
	ObjectNextElement,
	ObjectEnd,
	ArrayStart,
	ArrayNextElement,
	ArrayEnd,
	ElementNameStart,
	ElementNameEnd,
	ElementValueStart,
	StringValueStart,
	NumberValueStart,
	RealValueStart,
	BooleanTrueValueStart,
	BooleanFalseValueStart,
	NullValueStart,
	IntegerValueEnd,
	RealValueEnd,
	NullValueEnd,
	StringValueEnd,
	BooleanTrueValueEnd,
	BooleanFalseValueEnd,
	End,
	Error
};

class JsonTokenizer
{
public:
	JsonTokenizer(String source);

	JsonTokenType NextToken();

	uint32 GetPosition() const { return position_; }
	String& GetName() { return name_; }
	String& GetValue() { return name_; }
	int64 GetIntValue() const { return intVal_; }
	real64 GetRealVaue() const { return realVal_; }
	uint32 GetLine() const { return line_; }
private:
	uint32 line_;
	uint32 position_;
	int64 intVal_;
	real64 realVal_;
	Stack<JsonTokenizerContext> contextStack_;
	String name_;
	JsonTokenType lastToken_;
	String source_;
};