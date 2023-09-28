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
			void UpdateScriptingEngine();
			static void TestScriptingEngine();
			static void TestAddComponent();
			static void TestSpawnObject();

#pragma region MonoFunctionBindings

			// ECS
			static MonoString* BindCreateEntity(MonoString* name);
			static void BindAddComponent(MonoString* id, int componentType);
			static void BindRemoveComponent(MonoString* id, int componentType);

			// Test
			static void BindTestFunction();

			// Transform

			// Prefab

			// Parenting

			// Physics

			// Camera

			// Audio

			// Mesh Renderer
#pragma endregion

		private:
			static MonoDomain* s_RootDomain;
			static MonoDomain* s_AppDomain;
			static MonoAssembly* s_MonoAssembly;
	};

}