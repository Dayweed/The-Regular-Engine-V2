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
		Info.EnableEditor = true;
		return TRE_NEW EditorApplication(Info);
	}
}