#pragma once
#include "Core/Engine.h"

#define _CRTDBG_MAP_ALLOC
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

	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
}