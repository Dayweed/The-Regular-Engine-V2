#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "glm/glm.hpp"


//-----------------------------------------------------------------------------
// list of engine functions to bind to the scripting engine
//-----------------------------------------------------------------------------



namespace TRE
{

	

	class ScriptEngine 
	{
	public:
		ScriptEngine();
		~ScriptEngine();

		static void Init();
		static void Shutdown();

		static bool LoadAssembly(const std::string& assemblyPath);

		static void ReloadAssembly();

		static std::string TestGUID;
		static bool CreatedScriptObject;
		static void InitMono();
		static void ShutdownMono();
		static void UpdateScriptingEngine();
		static void TestScriptingEngine();
		static void TestAddComponent();
		static void TestSpawnObject();
		static void TestUpdataObject();
		static void SetTestGUID(std::string guid) { TestGUID = guid; }

		

	private:

		static MonoDomain* s_RootDomain;
		static MonoDomain* s_AppDomain;
		static MonoAssembly* s_MonoAssembly;
		static MonoObject* DemoObject;

		static MonoObject* InstantiateClass(MonoClass* monoClass);

		friend class ScriptBind;
	};

}