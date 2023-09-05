#include "pch.h"
#include "ECS.h"
#include "MemoryManager.h"
#include "Engine.h"
#include "Profiler.h"
#include "Physics/PhysicsSystem.h"
#include "Audio/AudioSystem.h"

//TO DELETE
#pragma region TO DELETE TEST
#include "Graphics/MeshRenderer.h"
#include "Graphics/Camera.h"
#include "Geom.h"
namespace TRE
{
	void DemoScene()
	{
		Geom::RunCompiler("../Assets/smooth_vase.desc");
		
		auto geom = Geom::Deserialize("../Assets/smooth_vase.geom");

		Entity test = ECSManager::Instance().CreateEntity();
		test->GetComponent<Properties>().m_Name = "Test";
		test->GetComponent<Transform>().m_Position.z = 25.f;
		test->GetComponent<Transform>().m_Scale = glm::vec3(20.f, 20.f, 20.f);
		test->GetComponent<Transform>().m_Rotation = glm::vec3(0.f, 0.f, 0.f);
		//std::shared_ptr<RenderObject> vase = RenderObject::CreateFromFile("../Assets/smooth_vase.obj");
		std::shared_ptr<RenderObject> vase = RenderObject::CreateFromGeom(std::move(geom));
		test->AddComponent<MeshRenderer>();
		test->GetComponent<MeshRenderer>().m_RenderObject = vase;

		Entity test2 = ECSManager::Instance().CreateEntity();
		test2->GetComponent<Properties>().m_Name = "Test2";
		test2->GetComponent<Transform>().m_Position.x = 2.f;
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
	}

	Engine::~Engine()
	{
	}

	void Engine::RegisterECS()
	{
		// Register Components
		ComponentManager::Instance().RegisterComponent<Undeployed>("Undeployed", true);
		ComponentManager::Instance().RegisterComponent<Removal>("Removal", true);
		ComponentManager::Instance().RegisterComponent<Properties>("Properties", true);
		ComponentManager::Instance().RegisterComponent<Transform>("Transform");
		ComponentManager::Instance().RegisterComponent<MeshRenderer>("Mesh Renderer");
		ComponentManager::Instance().RegisterComponent<Camera>("Camera");
		ComponentManager::Instance().RegisterComponent<SphereCollider>("SphereCollider");
		ComponentManager::Instance().RegisterComponent<BoxCollider>("BoxCollider");
		ComponentManager::Instance().RegisterComponent<Audio>("Audio");

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

		DemoScene();

		while (!m_Window->ShouldWindowClose())
		{

			m_Window->BeginFrame();

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