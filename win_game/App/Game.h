#pragma once
#include "UI/UINode.h"

class Game
{
public:
	virtual void Update(float delta) = 0;

	UINode* GetSceneRoot() const { return sceneRoot_; }
	bool IsDirty() const { return dirty_; };

protected:
	bool dirty_;
	UINode* sceneRoot_;
};
