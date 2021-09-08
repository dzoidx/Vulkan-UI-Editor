#pragma once
#include "Game.h"

class WindowControl;

class App
{
public:
	virtual void Init(WindowControl* winctl, Game* game) = 0;
	// ��������� ���� ���������� � ���������, ������� ����� ��� �����
	virtual void Start() = 0;
	// ������������� ���������� � ���������
	virtual void Stop() = 0;
};
