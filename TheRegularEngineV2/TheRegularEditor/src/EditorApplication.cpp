#include "EditorLayer.h"
#include "TREIncludes.h"
#include "EntryPoint.h"

namespace TRE
{
	class EditorApplication : public TRE::Engine
	{
		public:
			EditorApplication() : Engine()
			{
				_system_manager->RegisterSystem<EditorLayer>();
			}

		private:
	};

	TRE::Engine* TRE::CreateApp()
	{
		return new EditorApplication();
	}
}