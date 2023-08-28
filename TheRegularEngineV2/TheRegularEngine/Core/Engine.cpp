#include "pch.h"
#include "ECS.h"
#include "Engine.h"
#include "Physics/PhysicsSystem.h"

//TO DELETE
#pragma region TO DELETE TEST
#include "Graphics/MeshRenderer.h"
#include "Graphics/Camera.h"
#include "Graphics/Texture.h"
namespace TRE
{
	void DemoScene()
	{
		GO test = _ecs_manager->CreateGO();
		test->AddComponent<Properties>().m_Name = "Test";
		test->AddComponent<Transform>().m_Position.z = 25.f;
		test->GetComponent<Transform>().m_Scale = glm::vec3(20.f, 20.f, 20.f);
		test->GetComponent<Transform>().m_Rotation = glm::vec3(0.f, 0.f, 0.f);
		std::shared_ptr<RenderObject> vase = RenderObject::CreateFromFile("../Assets/smooth_vase.obj");
		test->AddComponent<MeshRenderer>();
		test->GetComponent<MeshRenderer>().m_RenderObject = vase;

		//GO test2 = _ecs_manager->CreateGO();
		//test2->AddComponent<Properties>().m_Name = "Test2";
		//test2->AddComponent<Transform>().m_Position.x = 20.f;
		//test2->GetComponent<Transform>().m_Position.z = 70.f;
		//test2->GetComponent<Transform>().m_Scale = glm::vec3(20.f, 20.f, 20.f);
		//test2->GetComponent<Transform>().m_Rotation = glm::vec3(0.f, 0.f, 45.f);
		//test2->AddComponent<MeshRenderer>();
		//test2->GetComponent<MeshRenderer>().m_RenderObject = vase;
		
		GO cam = _ecs_manager->CreateGO();
		cam->AddComponent<Properties>().m_Name = "cam";
		cam->AddComponent<Transform>().m_Position;
		cam->AddComponent<Camera>().m_Position = glm::vec3(0.0f, 0.0f, 0.0f);
		cam->GetComponent<Camera>().m_Rotation = glm::vec3(0.0f, 0.0f, 0.0f);

		_texture_manager->LoadTexture("../Assets/Test.png", "Test");

		_system_manager->GetSystem<CameraSystem>()->SetIsMainCamera(cam, true);
		// _system_manager->GetSystem<PhysicsSystem>()->ConstructSphereCollider(test2, { 4, 10, 4 }, 2);
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

		// Register Systems
		_system_manager->RegisterSystem<PhysicsSystem>();
		_system_manager->RegisterSystem<CameraSystem>();
	}

	void Engine::Update()
	{
		// To remove eventually
		_ecs_manager->TESTRUN();

		DemoScene();

		while (!m_Window->ShouldWindowClose())
		{
			m_Window->PollEvents();

			//Update



			//Draw
			m_Window->GetSwapChain().BeginFrame();
			m_Renderer->BeginFrame();

			if (m_EngineInfo.EnableEditor)
			{
				m_VulkanEditor->BeginFrame();
			}

			_system_manager->UpdateSystem();
			_system_manager->OnDestroyGO();
			_ecs_manager->DestroyRemovalGO();

			if (m_EngineInfo.EnableEditor)
			{
				m_VulkanEditor->EndFrame();
			}

			m_Window->SwapBuffers();
		}
	}

	void Engine::Shutdown()
	{
		//Some graphics class should call this, temporary
		_texture_manager->Shutdown();
		_system_manager->ShutdownSystem();
		_ecs_manager->DestroyAll();
	}
}