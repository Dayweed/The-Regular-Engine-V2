#include "pch.h"
#include "ECS.h"
#include "Engine.h"
#include "Profiler.h"
#include "Physics/PhysicsSystem.h"
#include "Audio/AudioSystem.h"

//TO DELETE
#pragma region TO DELETE TEST
#include "Graphics/MeshRenderer.h"
#include "Graphics/Camera.h"
#include "Graphics/Texture.h"
#include "Graphics/GeomCompiler.h"
namespace TRE
{
	void DemoScene()
	{
		_geom_compiler->Compile("../Assets/smooth_vase.obj");
		_geom_compiler->Serialize("../Assets/smooth_vase.geom");
		_geom_compiler->Deserialize("../Assets/smooth_vase.geom");

		GO test = _ecs_manager->CreateGO();
		test->AddComponent<Properties>().m_Name = "Test";
		test->AddComponent<Transform>().m_Position.z = 25.f;
		test->GetComponent<Transform>().m_Scale = glm::vec3(20.f, 20.f, 20.f);
		test->GetComponent<Transform>().m_Rotation = glm::vec3(0.f, 0.f, 0.f);
		//std::shared_ptr<RenderObject> vase = RenderObject::CreateFromFile("../Assets/smooth_vase.obj");
		std::shared_ptr<RenderObject> vase = RenderObject::CreateFromGeom(_geom_compiler->GetGeom());
		test->AddComponent<MeshRenderer>();
		test->GetComponent<MeshRenderer>().m_RenderObject = vase;

		GO test2 = _ecs_manager->CreateGO();
		test2->AddComponent<Properties>().m_Name = "Test2";
		test2->AddComponent<Transform>().m_Position.x = 2.f;
		test2->GetComponent<Transform>().m_Position.z = 50.f;
		test2->GetComponent<Transform>().m_Scale = glm::vec3(20.f, 20.f, 20.f);
		test2->GetComponent<Transform>().m_Rotation = glm::vec3(0.f, 0.f, 45.f);
		test2->AddComponent<MeshRenderer>();
		test2->GetComponent<MeshRenderer>().m_RenderObject = vase;
		
		GO cam = _ecs_manager->CreateGO();
		cam->AddComponent<Properties>().m_Name = "cam";
		cam->AddComponent<Transform>().m_Position;
		cam->AddComponent<Camera>().m_Position = glm::vec3(0.0f, 0.0f, 0.0f);
		cam->GetComponent<Camera>().m_Rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		cam->GetComponent<Camera>().m_Fov = 30.0f;

		//_texture_manager->LoadTexture("../Assets/Test.png", "Test");

		/*GO audio = _ecs_manager->CreateGO();
		audio->AddComponent<Audio>();
		audio->GetComponent<Audio>().m_IsPlaying = true;*/



		_system_manager->GetSystem<CameraSystem>()->SetIsMainCamera(cam, true);
		// _system_manager->GetSystem<PhysicsSystem>()->ConstructSphereCollider(test2, { 4, 10, 4 }, 2);
		//_system_manager->GetSystem<AudioSystem>()->LoadFile(audio);
		//_system_manager->GetSystem<AudioSystem>()->Play(audio, true);
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
		//m_SystemsManager = std::make_unique<SystemManager>();
		
		m_Renderer = std::make_shared<Renderer>(m_Window->GetRenderContext()->GetDevice());
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
		_component_manager->RegisterComponent<Removal>("Removal", true);
		_component_manager->RegisterComponent<Properties>("Properties", true);
		_component_manager->RegisterComponent<Transform>("Transform");
		_component_manager->RegisterComponent<MeshRenderer>("Mesh Renderer");
		_component_manager->RegisterComponent<Camera>("Camera");
		_component_manager->RegisterComponent<SphereCollider>("SphereCollider");
		_component_manager->RegisterComponent<BoxCollider>("BoxCollider");
		_component_manager->RegisterComponent<Audio>("Audio");

		// Register Systems
		_system_manager->RegisterSystem<PhysicsSystem>();
		_system_manager->RegisterSystem<CameraSystem>();
		_system_manager->RegisterSystem<AudioSystem>();
	}

	void Engine::Update()
	{
		// To remove eventually
		//_ecs_manager->TESTRUN();

		DemoScene();

		while (!m_Window->ShouldWindowClose())
		{
			m_Window->PollEvents();

			m_Window->GetSwapChain().BeginFrame();

			//Update
			if (m_EngineInfo.EnableEditor)
			{
				m_VulkanEditor->BeginFrame();
			}

			_profiler->StartTimer("Update");
			_system_manager->UpdateSystem();
			_system_manager->OnDestroyGO();
			_ecs_manager->DestroyRemovalGO();
			_profiler->EndTimer("Update");

			if (m_EngineInfo.EnableEditor)
			{
				m_VulkanEditor->EndFrame();
			}

			//Draw
			_profiler->StartTimer("Draw");
			m_Renderer->BeginFrame();

			m_Window->SwapBuffers();
			_profiler->EndTimer("Draw");

			// THIS IS COMMENTED OUT UNTIL IMGUI IS UP, iteration 1 would be used for displaying until IMGUI can use iteration 2
			//_profiler->PrintTimers();
		}
	}

	void Engine::Shutdown()
	{
		_system_manager->ShutdownSystem();
		_ecs_manager->DestroyAll();
	}
}