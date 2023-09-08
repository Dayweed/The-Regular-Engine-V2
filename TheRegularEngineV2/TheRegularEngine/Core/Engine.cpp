#include "pch.h"
#include "Engine.h"
#include "ECS.h"
#include "MemoryManager.h"
#include "Profiler.h"
#include "Physics/PhysicsSystem.h"
#include "Audio/AudioSystem.h"
#include "Logger.h"

//TO DELETE
#pragma region TO DELETE TEST
#include "Graphics/MeshRenderer.h"
#include "Graphics/Camera.h"
#include "Geom.h"
#include "Graphics/Shader.h"
namespace TRE
{
	void TestShader()
	{
		Shader MyShader;
		MyShader.LoadShader();
	}

	void AHHH()
	{
		Entity test = ECSManager::Instance().CreateEntity();
		test->GetComponent<Properties>().m_Name = "AHH";
		Entity ab = ECSManager::Instance().CreateEntity();
		ab->GetComponent<Properties>().m_Name = "CARLON";
		ab->AddComponent<FEL>().vec_i = { 4.5f, 2.f };
		ab->GetComponent<FEL>().nestedstruct.arr_c = '{';
	
		Entity fun = ECSManager::Instance().CreateEntity();
		fun->GetComponent<Properties>().m_Name = "fFNNN";
		fun->AddComponent<FEL>().arr_i[1] = 1.2f;

		std::cout << "- " << ECSManager::Instance().GetAllEntities().size() << "\n";
		for (Entity& obj : ECSManager::Instance().GetAllEntities())
		{
			std::cout << "= " << obj->GetName() << "|" << obj->HasComponent<Properties>() << "|" << obj->HasComponent<Parenting>() << "|" << obj->HasComponent<FEL>() << "\n";
		}
		std::string filePath{ECSManager::Instance().SaveEntities("Demo")};

		ECSManager::Instance().LoadEntities(filePath);
		std::cout << "- " << ECSManager::Instance().GetAllEntities().size() << "\n";
		for (Entity& obj : ECSManager::Instance().GetAllEntities())
		{
			std::cout << "= " << obj->GetName() << "|" << obj->HasComponent<Properties>() << "|" << obj->HasComponent<Parenting>() << "|" << obj->HasComponent<FEL>() << "\n";
			if (obj->HasComponent<FEL>())
			{
				std::cout << "== " << obj->GetComponent<FEL>().nestedstruct.arr_c << "\n";
				for (auto v : obj->GetComponent<FEL>().vec_i)
				{
					std::cout << "=== " << v << "\n";
				}
			}
		}
	}

	void DemoScene()
	{
		Geom::RunCompiler("../Assets/smooth_vase.desc");
		
		auto geom = Geom::Deserialize("../Assets/smooth_vase.geom");

		Entity test = ECSManager::Instance().CreateEntity();
		test->GetComponent<Properties>().m_Name = "Test";
		test->GetComponent<Transform>().m_Position.z = 25.f;
		test->GetComponent<Transform>().m_Scale = glm::vec3(20.f, 20.f, 20.f);
		test->GetComponent<Transform>().m_Rotation = glm::vec3(0.f, 0.f, 0.f);
		std::shared_ptr<RenderObject> vase = RenderObject::CreateFromGeom(std::move(geom));
		test->AddComponent<MeshRenderer>();
		test->GetComponent<MeshRenderer>().m_RenderObject = vase;

		Entity test2 = ECSManager::Instance().CreateEntity();
		test2->GetComponent<Properties>().m_Name = "Test2";
		test2->GetComponent<Transform>().m_Position.x = 2.f;
		test2->GetComponent<Transform>().m_Position.y = 10.f;
		test2->GetComponent<Transform>().m_Position.z = 50.f;
		test2->GetComponent<Transform>().m_Scale = glm::vec3(20.f, 20.f, 20.f);
		test2->GetComponent<Transform>().m_Rotation = glm::vec3(0.f, 0.f, 45.f);
		test2->AddComponent<MeshRenderer>();
		test2->GetComponent<MeshRenderer>().m_RenderObject = vase;
		
		Entity cam = ECSManager::Instance().CreateEntity();
		cam->GetComponent<Properties>().m_Name = "cam";
		cam->GetComponent<Transform>().m_Position;
		cam->AddComponent<Camera>().m_Position = glm::vec3(0.0f, 0.0f, 0.0f);
		cam->GetComponent<Camera>().m_Rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		cam->GetComponent<Camera>().m_Fov = 30.0f;

		//Entity audio = ECSManager::Instance().CreateEntity();
		//audio->AddComponent<Audio>();

		ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetIsMainCamera(cam, true);
		// _system_manager->GetSystem<PhysicsSystem>()->ConstructSphereCollider(test2, { 4, 10, 4 }, 2);
		//ECSSystemManager::Instance().GetSystem<AudioSystem>()->CompileAudio(audio);

		//ECSManager::Instance().SaveEntities("Demo.json");
	}
}
#pragma endregion TO DELETE TEST

namespace TRE
{
	Engine* Engine::s_Instance = nullptr;

	const std::shared_ptr<Window>& Engine::GetWindow()
	{
		return m_Window;
	}

	Engine& Engine::GetInstance()
	{
		return *s_Instance;
	}

	const std::shared_ptr<Renderer>& Engine::GetRenderer()
	{
		return m_Renderer;
	}

	const std::shared_ptr<VulkanEditor>& Engine::GetVulkanImgui()
	{
		return m_VulkanEditor;
	}

	Engine::Engine(const EngineInfo& EngineInfo)
	{
		s_Instance = this;
		m_EngineInfo = EngineInfo;
		m_Window = std::make_shared<Window>(m_EngineInfo.WindowConfigurations);
		
		m_Renderer = std::make_shared<Renderer>(m_Window->GetRenderContext()->GetDeviceInternally());
		m_Renderer->Initialize();

		if (m_EngineInfo.EnableEditor)
			m_VulkanEditor = std::make_shared<VulkanEditor>(m_Window->GetRenderContext()->GetDeviceInternally());

		TestShader();
	}

	Engine::~Engine()
	{
	}

	void Engine::RegisterECS()
	{
		// Register Components
		ComponentManager::Instance().RegisterComponent<Undeployed>("Undeployed", true);
		ComponentManager::Instance().RegisterComponent<Removal>("Removal", true);
		ComponentManager::Instance().RegisterComponent<Parenting>("Parenting", true);
		ComponentManager::Instance().RegisterComponent<Properties>("Properties", true);
		ComponentManager::Instance().RegisterComponent<Transform>("Transform");
		ComponentManager::Instance().RegisterComponent<MeshRenderer>("Mesh Renderer");
		ComponentManager::Instance().RegisterComponent<Camera>("Camera");
		ComponentManager::Instance().RegisterComponent<SphereCollider>("SphereCollider");
		ComponentManager::Instance().RegisterComponent<BoxCollider>("BoxCollider");
		ComponentManager::Instance().RegisterComponent<Audio>("Audio");
		ComponentManager::Instance().RegisterComponent<FEL>("FEL");

		// Register Systems
		ECSSystemManager::Instance().RegisterSystem<PhysicsSystem>();
		ECSSystemManager::Instance().RegisterSystem<CameraSystem>();
		ECSSystemManager::Instance().RegisterSystem<AudioSystem>();

		// Allocate Default Size for Memory Manager
		MemoryManager::Instance().AllocateEntitySize(MemoryManager::Instance().GetConfigSize());
	}

	void Engine::Update()
	{
		// To remove eventually
		//ECSManager::Instance().TESTRUN();
		//AHHH();
		DemoScene();

		while (!m_Window->ShouldWindowClose())
		{

			m_Window->BeginFrame();
			m_Window->UpdateDeltaTime();

			//Update
			Profiler::Instance().StartTimer("Update");
			ECSSystemManager::Instance().UpdateSystem();
			ECSSystemManager::Instance().OnDestroyEntities();
			ECSManager::Instance().DeleteRemovalEntities();
			Profiler::Instance().EndTimer("Update");
			m_Renderer->BeginFrame();

			// Imgui Update
			if (m_EngineInfo.EnableEditor)
			{
				Profiler::Instance().StartTimer("Imgui");
				m_VulkanEditor->BeginFrame();
				EditorSystemManager::Instance().UpdateSystem();
				m_VulkanEditor->EndFrame();
				Profiler::Instance().EndTimer("Imgui");
			}

			//Draw
			Profiler::Instance().StartTimer("Draw");

			m_Window->SwapBuffers();
			m_Window->PollEvents();
			Profiler::Instance().EndTimer("Draw");

			// THIS IS COMMENTED OUT UNTIL IMGUI IS UP, iteration 1 would be used for displaying until IMGUI can use iteration 2
			//Profiler::Instance().PrintTimers();
		}
	}

	void Engine::Shutdown()
	{
		ECSManager::Instance().DestroyAll();
		ECSSystemManager::Instance().ShutdownSystem();
		EditorSystemManager::Instance().ShutdownSystem();
		MemoryManager::Instance().DeleteEntities();
	}
}