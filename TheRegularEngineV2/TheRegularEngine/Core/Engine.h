#pragma once
#include "pch.h"
#include "Window.h"
#include "LayerSystem.h"

namespace TRE
{
	class Engine
	{
		public:
			virtual ~Engine();

			Engine(Engine&) = delete;
			void operator=(const Engine&) = delete;

			void Update();
			virtual void Shutdown();

			void AddSystem(Layer* system);

		protected:
			Engine();

		private:
			std::unique_ptr<Window> m_Window;
			LayerSystem* m_LayerSystems;
			static Engine* s_Instance;
	};

	Engine* CreateApp(); //Implemented by applications projects
}