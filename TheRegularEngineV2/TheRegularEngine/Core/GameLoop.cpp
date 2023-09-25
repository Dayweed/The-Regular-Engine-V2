#include "pch.h"

#include "GameLoop.h"

namespace TRE
{
	GameLoop& GameLoop::Instance()
	{
		static GameLoop instance;
		return instance;
	}
}