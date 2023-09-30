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

			static std::string TestGUID;
			static bool CreatedScriptObject;
			static void InitMono();
			static void ShutdownMono();
			static void BindFunctions();
			static void UpdateScriptingEngine();
			static void TestScriptingEngine();
			static void TestAddComponent();
			static void TestSpawnObject();

			static void SetTestGUID(std::string guid) { TestGUID = guid; }




#pragma region MonoFunctionBindings

			// ECS
			static MonoString* BindCreateEntity(MonoString* name);
			static void BindAddComponent(MonoString* id, int componentType);
			static void BindRemoveComponent(MonoString* id, int componentType);

			// Test
			static void BindTestFunction();
			static MonoString* BindGetTestGUID();

			// Transform

			static void BindSetPosition(MonoString* id, glm::vec3 newPos);
			static void BindSetRotation(MonoString* id, glm::vec3 newRot);

			static void BindGetPosition(MonoString* id, glm::vec3* result);
			static void BindGetRotation(MonoString* id, glm::vec3* result);


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