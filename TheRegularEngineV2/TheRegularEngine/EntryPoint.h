#pragma once
#include "Core/Engine.h"
#include "Core/Logger.h"

/* Replaces _CRTDBG_MAP_ALLOC											*/
/* This will actually make sure to sure to show where the leak is at	*/
#ifdef _DEBUG
#define DEBUG_MEMLEAK new( _CLIENT_BLOCK, __FILE__, __LINE__)
#define TRE_NEW DEBUG_MEMLEAK
#else
#define TRE_NEW new
#endif

#include <crtdbg.h>
#include <stdlib.h>

extern TRE::Engine* TRE::CreateApp(); //Getting the definition from other projects

#ifdef GAME
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	(void)hInstance;
	(void)hPrevInstance;
	(void)lpCmdLine;
	(void)nShowCmd;
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	TRE::Log::Init();
#endif
	TRE::Engine* App = TRE::CreateApp();
	App->Update();
	delete App;
}
#else
int main()
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	TRE::Log::Init();
#endif
	TRE::Engine* App = TRE::CreateApp();
	App->Update();
	delete App;
}
#endif