#include "pch.h"
#include "TREIncludes.h"
#include "EntryPoint.h"

namespace TRE
{
	class RuntimeApplication : public TRE::Engine
	{
		public:
			RuntimeApplication(const EngineInfo& EngineInfo) : Engine(EngineInfo)
			{
			
			}

		private:
	};

	TRE::Engine* TRE::CreateApp()
	{
		EngineInfo Info;
		Info.EnableEditor = true;
		return new RuntimeApplication(Info);
	}
}