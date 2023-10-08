#include "TREIncludes.h"
#include "EntryPoint.h"

namespace TRE
{
	class EditorApplication : public TRE::Engine
	{
		public:
			EditorApplication(const EngineInfo& EngineInfo) : Engine(EngineInfo)
			{
				
			}

		private:
	};

	TRE::Engine* TRE::CreateApp()
	{
		EngineInfo Info;
		Info.EnableEditor = true;
		return new EditorApplication(Info);
	}
}