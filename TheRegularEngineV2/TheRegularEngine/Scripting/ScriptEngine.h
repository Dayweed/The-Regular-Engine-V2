#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "glm/glm.hpp"


//-----------------------------------------------------------------------------
// list of engine functions to bind to the scripting engine
//-----------------------------------------------------------------------------

#include "EventSystem/Events/InputEvent.h"

namespace TRE
{

	enum class ScriptFieldType
	{
		None = 0,
		Float, Double,
		Bool, Char, Byte, Short, Int, Long,
		UnsignedByte, UnsignedShort, UnsignedInt, UnsignedLong,
		Vector2, Vector3, Vector4,
		Mat4,
		Entity

	};

	struct ScriptField
	{
		ScriptFieldType Type;
		std::string Name;

		MonoClassField* Field;
	};

	struct ScriptFieldInstance
	{
		ScriptField Field;

		ScriptFieldInstance()
		{
			memset(m_buffer, 0, sizeof(m_buffer));
		}

		template <typename T>
		T GetValue()
		{
			static_assert(sizeof(T) <= 16, "Type too Large!");
			return *(T*)m_buffer;
		}

		template<typename T>
		void SetValue(T value)
		{
			static_assert(sizeof(T) <= 16, "Type too Large!");
			memcpy(m_buffer, &value, sizeof(T));
		}


	private:
		uint8_t m_buffer[16];
		friend class ScriptEngine;

	};



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
		static void SetTestGUID(std::string guid) { TestGUID = guid; }

		static void DemoInit();
		static void DemoUpdate();

	private:
		static MonoObject* InstantiateClass(MonoClass* monoClass);

		friend class ScriptBind;
	};

}