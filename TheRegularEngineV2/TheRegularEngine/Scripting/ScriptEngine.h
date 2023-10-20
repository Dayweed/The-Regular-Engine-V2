#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "glm/glm.hpp"


//-----------------------------------------------------------------------------
// list of engine functions to bind to the scripting engine
//-----------------------------------------------------------------------------

#include "EventSystem/Events/InputEvent.h"



namespace TRE
{
	class ScriptInputHandler
	{
	public:
		ScriptInputHandler();
		~ScriptInputHandler();

		void Update();

		void GetKeyPressed(const InputEvent& event);

		int GetKey() { return _key; }
		int GetState() { return _state; }

	private:

		int _key{}, _state{};
		friend class ScriptEngine;
	};

	class ScriptEngine 
	{
	public:
		ScriptEngine();
		~ScriptEngine();

		void InitScriptingEngine();

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
		// Colliders
		static void BindResizeSphereCollider(MonoString* id, float s);
		static void BindResizeBoxCollider(MonoString* id, glm::vec3 s);
		static void BindAddForce(MonoString* id, glm::vec3 force);

		// Camera
		// Setters
		static void BindCamSetPosition(MonoString* id, glm::vec3 newPos);
		static void BindCamSetRotation(MonoString* id, glm::vec3 newRot);
		static void BindCamSetViewportSize(MonoString* id, glm::vec2 newSize);
		static void BindCamSetFocalPoint(MonoString* id, glm::vec3 focalpoint);
		static void BindCamSetFocalLength(MonoString* id, float focalLength);
		static void BindCamSetPitch(MonoString* id, float pitch);
		static void BindCamSetYaw(MonoString* id, float yaw);
		static void BindCamSetRoll(MonoString* id, float roll);
		static void BindCamSetFOV(MonoString* id, float fov);
		static void BindCamSetNear(MonoString* id, float n);
		static void BindCamSetFar(MonoString* id, float f);
		static void BindCamSetLeft(MonoString* id, float left);
		static void BindCamSetRight(MonoString* id, float right);
		static void BindCamSetTop(MonoString* id, float top);
		static void BindCamSetBottom(MonoString* id, float bottom);
		static void BindCamSetAspectRatio(MonoString* id, float aspectRatio);
		static void BindCamSetIsPerspective(MonoString* id, bool isPerspective);
		static void BindCamSetIsMainCamera(MonoString* id, bool isMainCamera);

		// Getters
		static void BindCamGetPosition(MonoString* id, glm::vec3* result);
		static void BindCamGetRotation(MonoString* id, glm::vec3* result);
		static void BindCamGetViewMatrix(MonoString* id, glm::mat4* result);
		static void BindCamGetProjectionMatrix(MonoString* id, glm::mat4* result);
		static void BindCamGetInverseViewMatrix(MonoString* id, glm::mat4* result);
		static void BindCamGetInverseProjectionMatrix(MonoString* id, glm::mat4* result);
		static void BindCamGetInverseViewProjectionMatrix(MonoString* id, glm::mat4* result);
		static void BindCamGetViewportSize(MonoString* id, glm::vec2* result);
		static void BindCamGetFocalPoint(MonoString* id, glm::vec3* result);
		static void BindCamGetFocalLength(MonoString* id, float* result);
		static void BindCamGetPitch(MonoString* id, float* result);
		static void BindCamGetYaw(MonoString* id, float* result);
		static void BindCamGetRoll(MonoString* id, float* result);
		static void BindCamGetFOV(MonoString* id, float* result);
		static void BindCamGetNear(MonoString* id, float* result);
		static void BindCamGetFar(MonoString* id, float* result);
		static void BindCamGetLeft(MonoString* id, float* result);
		static void BindCamGetRight(MonoString* id, float* result);
		static void BindCamGetTop(MonoString* id, float* result);
		static void BindCamGetBottom(MonoString* id, float* result);
		static void BindCamGetAspectRatio(MonoString* id, float* result);
		static void BindCamIsPerspective(MonoString* id, bool* result);
		static void BindCamIsMainCamera(MonoString* id, bool* result);

		// Audio

		// Mesh Renderer

		// Input Binding
		static bool BindGetKeyPressed(int key);
		static bool BindGetKeyTriggered(int key);

		// Logging
		static void SendMessageToConsole(MonoString* message);

#pragma endregion

	private:
		static MonoDomain* s_RootDomain;
		static MonoDomain* s_AppDomain;
		static MonoAssembly* s_MonoAssembly;
		static ScriptInputHandler* m_ScriptInputHandler;

	};

}