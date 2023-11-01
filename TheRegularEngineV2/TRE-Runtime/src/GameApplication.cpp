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
		Info.WindowConfigurations.Title = "Holey Moley";
		Info.WindowConfigurations.FullScreen = false;
		Info.EnableEditor = false;
		Info.MaximizeWindow = true;
		Info.EnableGame = true;
		return new RuntimeApplication(Info);
	}
}