#pragma once
#include "GameState.h"

#define START_PROFILE(name) uint64 name = GetCurrentNanos()
#define STOP_PROFILE(name, unit) Debug(String::Format("Unit '" #unit "' time: {0}ns.").Set(0, GetCurrentNanos() - name).ToUtf8())

extern GameState* currentGameState;

extern "C" GameState * InitEngine();
extern "C" void DeinitEngine(GameState * gameState);

void InitEngine(GameState* gs);
void DeinitEngine();
GameState* GetCurrentGameSate();

class EngineGuard
{
public:
	EngineGuard();
	~EngineGuard();
private:
	GameState* gameState_;
};