#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"


namespace TRE
{
	class ScriptEngine //Just me making sure the scriptcore works, implementer can replace all
	{
		public:
			static void InitMono();

		private:
			static MonoDomain* s_RootDomain;
			static MonoDomain* s_AppDomain;
			static MonoAssembly* s_MonoAssembly;
	};
}