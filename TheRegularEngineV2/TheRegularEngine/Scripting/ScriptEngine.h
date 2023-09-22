#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"

//-----------------------------------------------------------------------------
// list of engine functions to bind to the scripting engine
//-----------------------------------------------------------------------------



namespace TRE
{
	class ScriptEngine 
	{
		public:
			static void InitMono();
			static void ShutdownMono();
			static void BindFunctions();
			static void TestScriptingEngine();

		private:
			static MonoDomain* s_RootDomain;
			static MonoDomain* s_AppDomain;
			static MonoAssembly* s_MonoAssembly;
	};

	static void BindCreateEntity(MonoString* name);
}