#pragma once
#include "App/Game.h"
#include "App/WindowControl.h"
#include "Scene/Scene.h"

class ExoGame : public Game
{
public:
	ExoGame(WindowControl* win, const char* scenePath);

	virtual void Update(float delta);
private:
	WindowControl* winCtl_;
	String scenePath_;
	uint32 lastHash_;
	Scene lastScene_;
};
