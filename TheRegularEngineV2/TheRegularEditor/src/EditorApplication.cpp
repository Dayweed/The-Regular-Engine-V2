#include "TREIncludes.h"
#include "EntryPoint.h"

namespace TRE
{
	class EditorApplication : public TRE::Engine
	{
	public:
		EditorApplication() : Engine() 
		{

		}

	private:
	};

	TRE::Engine* TRE::CreateApp()
	{
		return new EditorApplication();
	}
}