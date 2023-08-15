#pragma once
#include "pch.h"
#include "Window.h"
#include "SystemManager.h"

namespace TRE
{
	class Engine
	{
		public:
			virtual ~Engine();

			Engine(Engine&) = delete;
			void operator=(const Engine&) = delete;

			void RegisterECS();
			void Update();
			virtual void Shutdown();

		protected:
			Engine();

		private:
			std::unique_ptr<Window> m_Window;
			static Engine* s_Instance;
	};

	Engine* CreateApp(); //Implemented by applications projects, used to seperate applications with engine
}