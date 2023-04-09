#pragma once

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
			static Engine* s_Instance;
	};

	Engine* CreateApp(); //Implemented by applications projects
}