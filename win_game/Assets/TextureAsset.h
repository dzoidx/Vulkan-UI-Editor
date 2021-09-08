#pragma once
#include "Asset.h"
#include "Sprite.h"
#include "Types.h"

// ���������� ������ �������.
	// ���������:
	//	4 ����� - ������
	//	4 ����� - ������
	//  1 ���� - ���� �����
	// ������ ������ = Width * Height * (Alpha ? 4 : 3)
class TextureAsset
{
public:
	TextureAsset() : Asset_() {}
	TextureAsset(Asset&& asset, uint32 id);
	TextureAsset(bool alpha, uint32 w, uint32 h, byte* data);

public:
	uint32 Id;
	uint32 Width;
	uint32 Height;
	uint32 BytesPerPixel;
	// ���� true �� � Data �� ������ ������� 4 �����, ����� 3
	bool Alpha;
	byte* Data;
	Asset Asset_;
	//uint32 TextureId; ����� ��������� � gpu ���������� ��������

	operator Sprite() const
	{
		return Sprite{ {0,0}, {1,1}, Id };
	}

	void LoadSync();

private:
	static const uint32 HeaderSize = 9;
};
