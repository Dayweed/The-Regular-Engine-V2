#include "pch.h"
#include "EditorSystem.h"
#include "TREIncludes.h"
#include "EntryPoint.h"

namespace TRE
{
	class EditorApplication : public TRE::Engine
	{
		public:
			EditorApplication(const EngineInfo& EngineInfo) : Engine(EngineInfo)
			{
				RegisterSystems<EditorSystem>();
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