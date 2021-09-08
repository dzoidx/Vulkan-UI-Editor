#pragma once
#include "Game.h"

class WindowControl;

class App
{
public:
	virtual void Init(WindowControl* winctl, Game* game) = 0;
	// запускает цикл обновления и отрисовки, создает поток для этого
	virtual void Start() = 0;
	// останавливает обновление и отрисовку
	virtual void Stop() = 0;
};
