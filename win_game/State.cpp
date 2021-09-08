#include "State.h"
#include "GameState.h"
#include "Logging.h"
#include "Memory/TempMemory.h"
#include "Scene/Timer.h"
#include "Types/String.h"
#include "Utils/StringBuilder.h"

GameState gameStateDummy{ 0, 0, new TempMemory(Megabytes(10)) };
GameState* currentGameState = &gameStateDummy;

void InitEngine(GameState* gs)
{
	currentGameState = gs;
	if (currentGameState == nullptr)
		gs->tempMemory = new TempMemory(Megabytes(10));

#ifdef _WIN32
	WinTimeInit();
#endif

	Debug(String::Format("Engine init.").ToUtf8());
}

GameState* InitEngine()
{
	GameState* result = (GameState*)malloc(sizeof(GameState));
	memset(result, 0, sizeof(GameState));
	result->tempMemory = new TempMemory(Megabytes(10));
	InitEngine(result);
	return result;
}

void DeinitEngine(GameState* gameState)
{
	Debug(String::Format("Engine deinit.").ToUtf8());
	delete gameState->tempMemory;
	delete gameState;
}

void DeinitEngine()
{
	assert(currentGameState != nullptr);
	DeinitEngine(currentGameState);
	currentGameState = nullptr;
}

GameState* GetCurrentGameSate()
{
	assert(currentGameState != nullptr);
	return currentGameState;
}

EngineGuard::EngineGuard()
{
	gameState_ = InitEngine();
}

EngineGuard::~EngineGuard()
{
	DeinitEngine(gameState_);
	gameState_ = nullptr;
}

