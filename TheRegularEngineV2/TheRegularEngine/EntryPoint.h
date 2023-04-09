#pragma once
#include "Core/Engine.h"

extern TRE::Engine* TRE::CreateApp(); //Getting the definition from other projects

int main()
{
	TRE::Engine* App = TRE::CreateApp();
	App->Update();
	App->Shutdown();
}