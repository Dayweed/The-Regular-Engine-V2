#include "pch.h"
#include "TREIncludes.h"
#include "EntryPoint.h"
#include "EditorSystem.h"

namespace TRE
{
	class EditorApplication : public TRE::Engine
	{
		public:
			EditorApplication(const EngineInfo& EngineInfo) : Engine(EngineInfo)
			{
				RegisterSystems<EditorSystem>();
			}
	};

	TRE::Engine* TRE::CreateApp()
	{
		EngineInfo Info;
		Info.WindowConfigurations.Title = "The Regular Editor";
		Info.WindowConfigurations.FullScreen = false;
		Info.EnableEditor = true;
		Info.MaximizeWindow = false;
		return TRE_NEW EditorApplication(Info);
	}
}