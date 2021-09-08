#include "ExoGame.h"

#include "Logging.h"
#include "State.h"
#include "Hash/Murmur32.h"
#include "IO/FileStream.h"
#include "Memory/TempMemory.h"
#include "Scene/SceneJson.h"
#include "Utils/StringBuilder.h"

real32 alignToKoeff[] =
{
		0,      // left
		0.5,    // center
		1,      // right
		0       // stretch
};

void RectToLocal(UINode* rect, const vector2& offset, const vector2& size, vector2* localOffset, vector2* localSize)
{
	real32 xBase = 0, yBase = 0;
	if (rect->AlignH != HorizontalAlign::Stretch)
		xBase = alignToKoeff[(int)rect->AlignH] * size.x;
	if (rect->AlignV != VerticalAlign::Stretch)
		yBase = alignToKoeff[(int)rect->AlignV] * size.y;

	vector2 sz = vector2((real32)rect->Size.X, (real32)rect->Size.Y);
	if (rect->AlignH == HorizontalAlign::Stretch)
		sz.x = size.x - sz.x - rect->Offset.X;
	else
		xBase -= sz.x * rect->Pivot.X;
	xBase += rect->Offset.X;

	if (rect->AlignV == VerticalAlign::Stretch)
		sz.y = size.y - sz.y - rect->Offset.Y;
	else
		yBase -= sz.y * rect->Pivot.Y;
	yBase += rect->Offset.Y;

	localOffset->x = xBase + offset.x;
	localOffset->y = yBase + offset.y;
	*localSize = sz;
}

void GenerateGeometry(vector2 offset, vector2 size, UIImage* node)
{
	vector2 localOffset, localSize;
	RectToLocal(node, offset, size, &localOffset, &localSize);

	vector2 vLU = localOffset;
	vector2 vLB(localOffset.x, localOffset.y + localSize.y);
	vector2 vRB(localOffset.x + localSize.x, localOffset.y + localSize.y);
	vector2 vRU(localOffset.x + localSize.x, localOffset.y);
	vector2 uvLU(0, 0);
	vector2 uvRU(1, 0);
	vector2 uvLB(0, 1);
	vector2 uvRB(1, 1);

	node->AABB.x = localOffset.x;
	node->AABB.y = localOffset.y;
	node->AABB.w = localSize.x;
	node->AABB.h = localSize.y;
	node->Geometry.Add(UIInputLayout(vLU, node->Color, uvLU));
	node->Geometry.Add(UIInputLayout(vLB, node->Color, uvLB));
	node->Geometry.Add(UIInputLayout(vRB, node->Color, uvRB));
	node->Geometry.Add(UIInputLayout(vRU, node->Color, uvRU));

	node->GeometryIndexes.Add(0);
	node->GeometryIndexes.Add(1);
	node->GeometryIndexes.Add(2);
	node->GeometryIndexes.Add(2);
	node->GeometryIndexes.Add(3);
	node->GeometryIndexes.Add(0);

	for (uint32 i = 0; i < node->Children.GetLen(); ++i)
		GenerateGeometry(localOffset, localSize, (UIImage*)node->Children[i]);
}

UIImage* CreateImage(Color color, int32 left, int32 top, int32 right, int32 bottom, HorizontalAlign alignH, VerticalAlign alignV, Position pivot, WindowControl* winCtl)
{
	UIImage* result = new UIImage();
	result->Color = color;
	result->Offset.X = left;
	result->Offset.Y = top;
	result->Size.X = right;
	result->Size.Y = bottom;
	result->AlignH = alignH;
	result->AlignV = alignV;
	result->Pivot = pivot;
	GenerateGeometry(vector2(0,0), vector2((real32)winCtl->GetWidth(), (real32)winCtl->GetHeight()), result);

	return result;
}

UIImage* CreateDummyScene(WindowControl* winCtl)
{
	UIImage* result = CreateImage(Color(0), 0, 0, 0, 0, HorizontalAlign::Stretch, VerticalAlign::Stretch, Position(), winCtl);

	UIImage* footer = CreateImage(Color(.5f), 0, 0, 0, 100, HorizontalAlign::Stretch, VerticalAlign::Bottom, Position(0, 1), winCtl);

	result->Children.Add(footer);

	return result;
}

ExoGame::ExoGame(WindowControl* win, const char* scenePath)
{
	winCtl_ = win;
	if (scenePath == nullptr)
		scenePath_ = L"";
	else
		scenePath_ = scenePath;
	lastHash_ = 0;
}

void ExoGame::Update(float delta)
{
	if (!scenePath_.IsEmpty() && (GetCurrentGameSate()->FrameCount == 0 || GetCurrentGameSate()->FrameCount % 30 == 0))
	{
		FileStream s = FileStream::OpenRead(scenePath_);
		uint32 fileSize = (uint32)s.Length_;
		byte* buff = GetCurrentGameSate()->tempMemory->Allocate<byte>(fileSize);
		fileSize = s.Read(buff, fileSize);
		s.Close();
		uint32 hash = HashMurmur32(buff, fileSize, 0);
		if (hash != lastHash_)
		{
			lastHash_ = hash;
			Debug(String::Format("Scene change detected. frame={0},hash={1}.")
				.Set(0, GetCurrentGameSate()->FrameCount)
				.Set(1, hash)
				.ToUtf8());
			String data((char*)buff);
			SceneJson sceneJson(data);
			if (sceneJson.IsValid())
			{
				lastScene_ = (Scene&&)sceneJson.GetScene();
				UIImage* n = lastScene_.Nodes[0]; // TODO: избавиться от листа нод в руте
				sceneRoot_ = n;
				GenerateGeometry(vector2(0,0), vector2((real32)winCtl_->GetWidth(), (real32)winCtl_->GetHeight()), n);
				dirty_ = true;
			}
		}
	}
	else if (GetCurrentGameSate()->FrameCount == 0)
	{
		Debug("Creating dummy scene.");
		lastScene_ = Scene{};
		lastScene_.Nodes.Add(CreateDummyScene(winCtl_));
		sceneRoot_ = lastScene_.Nodes[0];
		dirty_ = true;
	}
	else
		dirty_ = false;
	++GetCurrentGameSate()->FrameCount;
}