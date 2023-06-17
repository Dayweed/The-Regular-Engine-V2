#include "TREIncludes.h"
#include "EntryPoint.h"
#include "EditorLayer.h"

namespace TRE
{
	class EditorApplication : public TRE::Engine
	{
		public:
			EditorApplication() : Engine()
			{
				AddSystem(new EditorLayer);
			}

		private:
	};

	TRE::Engine* TRE::CreateApp()
	{
		return new EditorApplication();
	}
}