#pragma once
#include "Types.h"
#include "Formats/Math.h"
#include "Types/Color.h"
#include "Types/List.h"

class UIImage;

struct UIUniformLayout
{
	matrix4x4 view;
};

struct UIInputLayout
{
	vector4 pos;
	Color color;
	vector2 uv;

	UIInputLayout() : pos(), color(), uv() {}
	UIInputLayout(vector4 p, Color c) :pos(p), color(c), uv() {}
	UIInputLayout(vector4 p, Color c, vector2 u) :pos(p), color(c), uv(u) {}
};

enum class UINodeType
{
	Base,
	Image
};

class Position
{
public:
	int32 X;
	int32 Y;

	Position() : Position(0, 0) {}
	Position(int32 x, int32 y) : X(x), Y(y) {}

	friend bool operator==(const Position& f, const Position& s) { return f.X == s.X && f.Y == s.Y; }
	friend bool operator!=(const Position& f, const Position& s) { return f.X != s.X || f.Y != s.Y; }
};

enum class HorizontalAlign
{
	Left,
	Center,
	Right,
	Stretch
};

enum class VerticalAlign
{
	Top,
	Middle,
	Bottom,
	Stretch
};

class Line
{
public:
	vector2 from;
	vector2 to;
	color color;
	real32 width;
};

struct Quad // 24 bytes
{
	real32 X, Y;
	Color Color;
};

struct Batch
{
	int32 Amount;
	int32 TextureId;
};

struct FlatScene
{
	List<Quad> Quads;
	List<Batch> Batches;
};

class UINode
{
public:
	UINodeType Type;
	// 0,0 is upper left corner of node's parent
	Position Offset;
	// Размер элемента. 
	// Если элемент растягивается то соответсвующие оси это отступ от края.
	Position Size;
	// смещение на центр. используется для выравнивания элемента.
	Position Pivot;
	HorizontalAlign AlignH;
	VerticalAlign  AlignV;
	// указание рендереру что элемент нужно перерисовать, перегенерировать геометрию и т.п.
	bool Dirty; // @ignore
	// все дочерние элементы обрезаются по прямоуголнику этого элемента
	bool Mask;
	// вложенные элементы
	List<UINode*> Children; // @ignore
	aabb AABB;

	virtual ~UINode()
	{
	    for(int32 i = 0; i < Children.GetLen(); ++i)
	    {
	        (*Children[i]).~UINode();
	    }
	}

	List<UIInputLayout> Geometry; // @ignore
	List<uint32> GeometryIndexes; // @ignore

	static bool CompareScenes(UINode* root1, UINode* root2);
};