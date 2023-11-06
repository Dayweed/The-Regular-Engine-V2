#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "glm/glm.hpp"


//-----------------------------------------------------------------------------
// list of engine functions to bind to the scripting engine
//-----------------------------------------------------------------------------

#include "Core/ECS.h"

namespace TRE
{
	using CSEntityID = unsigned long long;        // C# EntityID

	enum class ScriptFieldTypes
	{
		None = 0,
		Float, Double,
		Boolean, Char, Byte, Short, Int, Long,
		UnsignedChar, UnsignedShort, UnsignedInt, UnsignedLong,
		Vector2, Vector3, Vector4,
		Entity, String
	};

	struct ScriptField
	{
		ScriptFieldTypes m_Type;
		std::string m_Name;

		MonoClassField* m_MonoField;
	};

	struct ScriptFieldInstance
	{
		ScriptField m_Field;

		ScriptFieldInstance()
		{
			memset(&m_Field, 0, sizeof(ScriptField));
		}

		template<typename T>
		T GetValue()
		{
			static_assert(sizeof(T) <= 16, "Type too large!");
			return *(T*)m_buffer;
		}

		template<typename T>
		void SetValue(T value)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");
			memcpy(m_buffer, &value, sizeof(T));

		}

	private:

		uint8_t m_buffer[16];

		friend class ScriptInstance;
		friend class ScriptEngine;

	};

	using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;// Store the fields of the class

	class ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace,const std::string& className, bool isCore);

		MonoObject* Instantiate();
		MonoMethod* GetMethod(const std::string& name, int paramCount);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params);
		MonoClass* GetMonoClass();

		std::map<std::string, ScriptField>& GetFields() { return m_Fields; }


	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;
		MonoClass* m_MonoClass = nullptr;

		//map of the fields in the class
		std::map<std::string, ScriptField> m_Fields;

		friend class ScriptEngine;

	};

	class ScriptInstance
	{
	public:

		ScriptInstance(std::shared_ptr<ScriptClass> scriptClass, std::string entity);
		~ScriptInstance();

		void OnEnableInvoke();
		void OnDisableInvoke();
		void OnDestroyInvoke();
		void OnCreateInvoke();
		void OnStartInvoke();
		void OnUpdateInvoke();
		void OnLateUpdateInvoke();
		void OnTriggerStayInvoke(Entity other);
		void OnCollisionStayInvoke(Entity other);

		std::shared_ptr<ScriptClass> GetScriptClass() { return m_ScriptClass; }

		MonoObject* GetScriptObject() { return m_Instance; }

		template<typename T>
		T GetFieldValue(const std::string& name)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");

			bool success = GetInternalFieldValue(name, s_fieldBuffer);
			if(!success)
				return T();
			return *(T*)s_fieldBuffer;
		}

		template<typename T>
		void SetFieldValue(const std::string& name, T value)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");

			SetInternalFieldValue(name, &value);
		}

	private:

		bool GetInternalFieldValue(const std::string& name, void* buffer);
		bool SetInternalFieldValue(const std::string& name, const void* buffer);

		// This is the class that this instance is based on
		std::shared_ptr<ScriptClass> m_ScriptClass;
		// Store the instance of the class that is created by ScriptClass
		MonoObject* m_Instance = nullptr;
		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_EnableMethod = nullptr;
		MonoMethod* m_DisableMethod = nullptr;
		MonoMethod* m_DestroyMethod = nullptr;
		MonoMethod* m_CreateMethod = nullptr;
		MonoMethod* m_StartMethod = nullptr;
		MonoMethod* m_UpdateMethod = nullptr;
		MonoMethod* m_LateUpdateMethod = nullptr;
		MonoMethod* m_TriggerStayMethod = nullptr;
		MonoMethod* m_CollisionStayMethod = nullptr;

		std::uint32_t m_GCHandle{};

		inline static char s_fieldBuffer[16];

		friend class ScriptEngine;
		friend struct ScriptFieldInstance;

	};

	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* MonoCoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		MonoAssembly* MonoProjectAssembly = nullptr;
		MonoImage* ProjectAssemblyImage = nullptr;

		MonoObject* DemoObject = nullptr;

		ScriptClass MainClass;

		std::unordered_map<std::string, std::shared_ptr<ScriptClass>> ScriptClasses;
		std::unordered_map<std::string, std::shared_ptr<ScriptInstance>> ScriptInstances;
		std::unordered_map<std::string, ScriptFieldMap> EntityFieldMap;

		std::string MonoAssemblyPath;
		std::string MonoProjectPath;

	};

	class ScriptEngine 
	{
	public:
		static ScriptEngineData* s_ScriptEngineData;

		static void Init();
		static void Shutdown();

		static bool LoadAssembly(const std::string& assemblyPath);
		static bool LoadProjectAssembly(const std::string& projectPath);
		static void LoadClassesFromAssembly();

		static bool RecompileScripts();
		static void ReloadAssembly();

		static void CreateCSEntityData(Entity e);

		static void InitScriptingMain();
		static void UpdateScriptingMain();

		static bool EntityClassExists(const std::string& className);
		static void OnEnableEntity(Entity e);
		static void OnDisableEntity(Entity e);
		static void OnDestroyEntity(Entity e);
		static void OnCreateEntity(Entity e);
		static void OnStartEntity(Entity e);
		static void OnUpdateEntity(Entity e	);
		static void OnLateUpdateEntity(Entity e	);

		// Collision
		static void OnTriggerStay(Entity e, Entity other);
		static void OnCollisionStay(Entity e, Entity other);

		static void PrintAllContainersHere();

		static MonoString* CreateMonoString(const std::string& guid);

		//Getter functions to obtain data from scriptEngineData

		static MonoObject* GetManagedInstance(std::string GUID);

		static std::shared_ptr<ScriptInstance> GetEntityInstance(std::string GUID);


	private:
		static void InitMono();
		static void ShutdownMono();

		static MonoObject* InstantiateClass(MonoClass* monoClass);

		friend class ScriptBind;
		friend class ScriptClass;
		friend class ScriptComponent;
		
	};

}