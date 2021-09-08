#include "SceneJson.h"

SceneJson::SceneJson(String& data)
	:tokenizer_(data), _invalid(), _errorLine()
{
	Load();
}

void SceneJson::Error()
{
	_invalid = true;
	_errorLine = tokenizer_.GetLine();
}

void SceneJson::Load()
{
	enum class State
	{
		None,
		MainFieldStart,
		ReadSceneName,
		ReadSceneNodes,
		ReadNextNode,
		ReadNodeElementName,
		ReadNodeType,
		ReadNodeColor,
		ReadNodeOffset,
		ReadNodeSize,
		ReadNodePivot,
		ReadNodeHAlign,
		ReadNodeVAlign,
		ReadNodeMask,
		ReadNodeChildren,
		ReadNodeNextChild,
		ReadPositionElement,
		ReadPositionX,
		ReadPositionY,
		End
	};
	struct NodeElCounts
	{
		uint32 type;
		uint32 color;
		uint32 offset;
		uint32 size;
		uint32 pivot;
		uint32 halign;
		uint32 valign;
		uint32 position_x;
		uint32 position_y;
		uint32 children;
		uint32 mask;
	};
	State state{};
	String name;
	uint32 nameElCount = 0;
	uint32 nodesElCount = 0;
	List<NodeElCounts> nodeCounts;
	JsonTokenType token = tokenizer_.NextToken();
	List<UIImage*> nodes;
	Position* position = nullptr;
	while (token != JsonTokenType::Error && token != JsonTokenType::End)
	{
		switch (state)
		{
		case State::None:
			if (token != JsonTokenType::ObjectStart)
			{
				Error();
				return;
			}
			state = State::MainFieldStart;
			break;
		case State::MainFieldStart:
			if (token == JsonTokenType::ElementNameEnd)
			{
				name = tokenizer_.GetName();
				if (name == L"name")
				{
					if (nameElCount++ > 0)
					{
						Error();
						return;
					}
					state = State::ReadSceneName;
				}
				else if (name == L"nodes")
				{
					if (nodesElCount++ > 0)
					{
						Error();
						return;
					}
					state = State::ReadSceneNodes;
				}
				else
				{
					Error();
					return;
				}
			}
			else if (token != JsonTokenType::ElementNameStart && token != JsonTokenType::ObjectEnd && token != JsonTokenType::ObjectNextElement)
			{
				Error();
				return;
			}
			break;
		case State::ReadSceneName:
			if (token == JsonTokenType::StringValueEnd)
			{
				scene_.Name = tokenizer_.GetValue();
				if (nameElCount && nodesElCount)
					state = State::End;
				else
					state = State::MainFieldStart;
			}
			else if (token != JsonTokenType::ElementValueStart && token != JsonTokenType::StringValueStart)
			{
				Error();
				return;
			}
			break;
		case State::ReadSceneNodes:
			if (token == JsonTokenType::ArrayStart)
			{
				state = State::ReadNextNode;
			}
			else if (token != JsonTokenType::ElementValueStart)
			{
				Error();
				return;
			}
			break;
		case State::ReadNodeChildren:
			if (token == JsonTokenType::ArrayStart)
			{
				state = State::ReadNodeNextChild;
			}
			else if (token != JsonTokenType::ElementValueStart)
			{
				Error();
				return;
			}
			break;
		case State::ReadNextNode:
			if (token == JsonTokenType::ObjectStart)
			{
				state = State::ReadNodeElementName;
				nodeCounts.Add(NodeElCounts{});
				nodes.Add(new UIImage());
			}
			else if (token == JsonTokenType::ArrayEnd)
			{
				state = State::MainFieldStart;
			}
			else if (token != JsonTokenType::ArrayNextElement)
			{
				Error();
				return;
			}
			break;
		case State::ReadNodeNextChild:
			if (token == JsonTokenType::ObjectStart)
			{
				state = State::ReadNodeElementName;
				nodeCounts.Add(NodeElCounts{});
				nodes.Add(new UIImage());
			}
			else if (token == JsonTokenType::ArrayEnd)
			{
				state = State::ReadNodeElementName;
			}
			else if (token != JsonTokenType::ArrayNextElement)
			{
				Error();
				return;
			}
			break;
		case State::ReadNodeElementName:
			if (token == JsonTokenType::ElementNameEnd)
			{
				name = tokenizer_.GetName();
				if (name == L"type")
				{
					if (nodeCounts.Last().type++ > 0)
					{
						Error();
						return;
					}
					state = State::ReadNodeType;
				}
				else if (name == L"color")
				{
					if (nodeCounts.Last().color++ > 0)
					{
						Error();
						return;
					}
					state = State::ReadNodeColor;
				}
				else if (name == L"offset")
				{
					if (nodeCounts.Last().offset++ > 0)
					{
						Error();
						return;
					}
					state = State::ReadNodeOffset;
				}
				else if (name == L"size")
				{
					if (nodeCounts.Last().size++ > 0)
					{
						Error();
						return;
					}
					state = State::ReadNodeSize;
				}
				else if (name == L"pivot")
				{
					if (nodeCounts.Last().pivot++ > 0)
					{
						Error();
						return;
					}
					state = State::ReadNodePivot;
				}
				else if (name == L"halign")
				{
					if (nodeCounts.Last().halign++ > 0)
					{
						Error();
						return;
					}
					state = State::ReadNodeHAlign;
				}
				else if (name == L"valign")
				{
					if (nodeCounts.Last().valign++ > 0)
					{
						Error();
						return;
					}
					state = State::ReadNodeVAlign;
				}
				else if (name == L"children")
				{
					if (nodeCounts.Last().children++ > 0)
					{
						Error();
						return;
					}
					state = State::ReadSceneNodes;
				}
				else if (name == L"mask")
				{
					if (nodeCounts.Last().mask++ > 0)
					{
						Error();
						return;
					}
					state = State::ReadNodeMask;
				}
				else
				{
					Error();
					return;
				}
			}
			else if (token == JsonTokenType::ObjectEnd)
			{
				UIImage* n = nodes.RemoveLast();
				nodeCounts.RemoveLast();
				if (nodes.GetLen() == 0)
				{
					scene_.Nodes.Add(n);
					state = State::ReadNextNode;
				}
				else
				{
					nodes.Last()->Children.Add(n);
					state = State::ReadNodeNextChild;
				}
			}
			else if (token != JsonTokenType::ElementNameStart && token != JsonTokenType::ObjectNextElement)
			{
				Error();
				return;
			}
			break;
		case State::ReadNodeType:
			if (token == JsonTokenType::StringValueEnd)
			{
				if (tokenizer_.GetValue() == L"image")
				{
					nodes.Last()->Type = UINodeType::Image;
					state = State::ReadNodeElementName;
				}
				else
				{
					Error();
					return;
				}
			}
			else if (token != JsonTokenType::ElementValueStart && token != JsonTokenType::StringValueStart)
			{
				Error();
				return;
			}
			break;
		case State::ReadNodeColor:
			if (token == JsonTokenType::StringValueEnd)
			{
				nodes.Last()->Color = Color::Color(tokenizer_.GetValue());
				state = State::ReadNodeElementName;
			}
			else if (token != JsonTokenType::ElementValueStart && token != JsonTokenType::StringValueStart)
			{
				Error();
				return;
			}
			break;
		case State::ReadNodeOffset:
			if (token == JsonTokenType::ObjectStart)
			{
				nodeCounts.Last().position_x = 0;
				nodeCounts.Last().position_y = 0;
				nodes.Last()->Offset = Position{};
				position = &nodes.Last()->Offset;
				state = State::ReadPositionElement;
			}
			else if (token != JsonTokenType::ElementValueStart)
			{
				Error();
				return;
			}
			break;
		case State::ReadNodeSize:
			if (token == JsonTokenType::ObjectStart)
			{
				nodeCounts.Last().position_x = 0;
				nodeCounts.Last().position_y = 0;
				nodes.Last()->Size = Position{};
				position = &nodes.Last()->Size;
				state = State::ReadPositionElement;
			}
			else if (token != JsonTokenType::ElementValueStart)
			{
				Error();
				return;
			}
			break;
		case State::ReadNodePivot:
			if (token == JsonTokenType::ObjectStart)
			{
				nodeCounts.Last().position_x = 0;
				nodeCounts.Last().position_y = 0;
				nodes.Last()->Pivot = Position{};
				position = &nodes.Last()->Pivot;
				state = State::ReadPositionElement;
			}
			else if (token != JsonTokenType::ElementValueStart)
			{
				Error();
				return;
			}
			break;
		case State::ReadNodeMask:
			if (token == JsonTokenType::BooleanFalseValueEnd)
			{
				nodes.Last()->Mask = false;
				state = State::ReadNodeElementName;
			}
			else if (token == JsonTokenType::BooleanTrueValueEnd)
			{
				nodes.Last()->Mask = true;
				state = State::ReadNodeElementName;
			}
			else if (token != JsonTokenType::ElementValueStart && token != JsonTokenType::BooleanFalseValueStart && token != JsonTokenType::BooleanTrueValueStart)
			{
				Error();
				return;
			}
			break;
		case State::ReadPositionElement:
			if (token == JsonTokenType::ElementNameEnd)
			{
				name = tokenizer_.GetName();
				if (name == L"x")
				{
					if (nodeCounts.Last().position_x++ > 0)
					{
						Error();
						return;
					}
					state = State::ReadPositionX;
				}
				else if (name == L"y")
				{
					if (nodeCounts.Last().position_y++ > 0)
					{
						Error();
						return;
					}
					state = State::ReadPositionY;
				}
				else
				{
					Error();
					return;
				}
			}
			else if (token == JsonTokenType::ObjectEnd)
			{
				state = State::ReadNodeElementName;
			}
			else if (token != JsonTokenType::ElementNameStart && token != JsonTokenType::ObjectNextElement)
			{
				Error();
				return;
			}
			break;
		case State::ReadPositionX:
			if (token == JsonTokenType::IntegerValueEnd)
			{
				position->X = (int32)tokenizer_.GetIntValue();
				state = State::ReadPositionElement;
			}
			else if (token != JsonTokenType::ElementValueStart && token != JsonTokenType::NumberValueStart)
			{
				Error();
				return;
			}
			break;
		case State::ReadPositionY:
			if (token == JsonTokenType::IntegerValueEnd)
			{
				position->Y = (int32)tokenizer_.GetIntValue();
				state = State::ReadPositionElement;
			}
			else if (token != JsonTokenType::ElementValueStart && token != JsonTokenType::NumberValueStart)
			{
				Error();
				return;
			}
			break;
		case State::ReadNodeHAlign:
			if (token == JsonTokenType::StringValueEnd)
			{
				name = tokenizer_.GetValue();
				if (name == L"stretch")
				{
					nodes.Last()->AlignH = HorizontalAlign::Stretch;
				}
				else if (name == L"center")
				{
					nodes.Last()->AlignH = HorizontalAlign::Center;
				}
				else if (name == L"left")
				{
					nodes.Last()->AlignH = HorizontalAlign::Left;
				}
				else if (name == L"right")
				{
					nodes.Last()->AlignH = HorizontalAlign::Right;
				}
				else
				{
					Error();
					return;
				}
				state = State::ReadNodeElementName;
			}
			else if (token != JsonTokenType::ElementValueStart && token != JsonTokenType::StringValueStart)
			{
				Error();
				return;
			}
			break;
		case State::ReadNodeVAlign:
			if (token == JsonTokenType::StringValueEnd)
			{
				name = tokenizer_.GetValue();
				if (name == L"stretch")
				{
					nodes.Last()->AlignV = VerticalAlign::Stretch;
				}
				else if (name == L"middle")
				{
					nodes.Last()->AlignV = VerticalAlign::Middle;
				}
				else if (name == L"top")
				{
					nodes.Last()->AlignV = VerticalAlign::Top;
				}
				else if (name == L"bottom")
				{
					nodes.Last()->AlignV = VerticalAlign::Bottom;
				}
				else
				{
					Error();
					return;
				}
				state = State::ReadNodeElementName;
			}
			else if (token != JsonTokenType::ElementValueStart && token != JsonTokenType::StringValueStart)
			{
				Error();
				return;
			}
			break;
		default:
			Error();
			return;
		}
		token = tokenizer_.NextToken();
	}
	if (nodes.GetLen() != 0)
	{
		Error();
	}
}