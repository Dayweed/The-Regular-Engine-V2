#pragma once
#include "pch.h"
#include "Window.h"

namespace TRE
{
	class Engine
	{
		public:
			Engine();
			virtual ~Engine();

			Engine(Engine&) = delete;
			void operator=(const Engine&) = delete;

			void Update();
			virtual void Shutdown();

		protected:

		private:
			std::unique_ptr<Window> m_Window;
			
			
			static Engine* s_Instance;
	};

	Engine* CreateApp(); //Implemented by applications projects
}