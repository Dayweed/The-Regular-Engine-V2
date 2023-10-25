#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "glm/glm.hpp"


//-----------------------------------------------------------------------------
// list of engine functions to bind to the scripting engine
//-----------------------------------------------------------------------------



namespace TRE
{

	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* MonoAssembly = nullptr;

		MonoImage* AssemblyImage = nullptr;

		MonoObject* DemoObject = nullptr;

		std::string MonoAssemblyPath;

	};



	class ScriptEngine 
	{
	public:

		static void Init();
		static void Shutdown();

		static bool LoadAssembly(const std::string& assemblyPath);

		static void ReloadAssembly();

		
		

		static std::string TestGUID;
		static bool CreatedScriptObject;
		
		
		static void UpdateScriptingEngine();
		static void TestScriptingEngine();
		static void TestAddComponent();

		static void TestUpdateObject();
		static void SetTestGUID(std::string guid) { TestGUID = guid; }

		

	private:
		static void InitMono();
		static void ShutdownMono();
		static ScriptEngineData* s_ScriptEngineData;

		static MonoObject* InstantiateClass(MonoClass* monoClass);

		friend class ScriptBind;
		
	};

}