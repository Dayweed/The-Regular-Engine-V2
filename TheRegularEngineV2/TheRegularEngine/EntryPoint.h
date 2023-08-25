#pragma once
#include "Core/Engine.h"

/* Replaces _CRTDBG_MAP_ALLOC											*/
/* This will actually make sure to sure to show where the leak is at	*/
#ifdef _DEBUG
#define DEBUG_MEMLEAK new( _CLIENT_BLOCK, __FILE__, __LINE__)
#define new DEBUG_MEMLEAK
#endif

#include <crtdbg.h>
#include <stdlib.h>

extern TRE::Engine* TRE::CreateApp(); //Getting the definition from other projects

int main()
{
	#if defined(DEBUG) | defined(_DEBUG)
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	TRE::Engine* App = TRE::CreateApp();
	App->RegisterECS();
	App->Update();
	App->Shutdown();
	delete App;
}