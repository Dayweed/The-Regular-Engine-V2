#pragma once
#include "pch.h"

namespace TRE
{
	class GameLoop
	{
	public:
		static GameLoop& Instance();

	private:
		// Delete possible copy ctor and assignment to ensure singleton
		GameLoop() {};
		GameLoop(GameLoop const&) = delete;
		void operator=(GameLoop const&) = delete;
		void* operator new(size_t) = delete;
	};
}