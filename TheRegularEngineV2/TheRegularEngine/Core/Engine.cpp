#include "pch.h"
#include "TREIncludes.h"
#include "Engine.h"
#include "ECS.h"
#include "Transform.h"
#include "SceneManager.h"
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
#include <time.h>       /* time */
#include "Assets/AssetManager.h"

namespace TRE
{
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
		std::string fileName{ "AHHHScene" };
		SceneManager::Instance().SaveSceneAs(fileName);

		std::cout << "File Name: > " << fileName << "\n";

		SceneManager::Instance().LoadScene(fileName);
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
		Geom::RunCompiler("../Assets/mine.desc");
		std::unique_ptr<RenderObject> ro = RenderObject::CreateFromGeom((Geom::Deserialize("../Assets/mine.geom")));
		ro->SetHandle(1);
		AssetManager::Instance().AddAsset(std::move(ro));

		auto transformSystem = ECSSystemManager::Instance().GetSystem<TransformSystem>();
		auto meshRendererSystem = ECSSystemManager::Instance().GetSystem<MeshRendererSystem>();
		auto cameraSystem = ECSSystemManager::Instance().GetSystem<CameraSystem>();

		Entity test2 = ECSManager::Instance().CreateEntity();
		test2->GetComponent<Properties>().m_Name = "Test2";
		transformSystem->SetPosition(test2, glm::vec3(30.f, 10.f, 100.f));
		transformSystem->SetScale(test2, glm::vec3(5.f, 5.f, 5.f));
		transformSystem->SetRotation(test2, glm::vec3(0.f, 0.f, 45.f));
		test2->AddComponent<MeshRenderer>();
		meshRendererSystem->SetMeshRenderer(test2, AssetManager::Instance().GetAsset<RenderObject>(1));

		Entity test = ECSManager::Instance().CreateEntity();
		test->GetComponent<Properties>().m_Name = "Test";
		transformSystem->SetPosition(test, glm::vec3(0.f,0.f, 25.f));
		transformSystem->SetScale(test, glm::vec3(5.f, 5.f, 5.f));
		test->AddComponent<MeshRenderer>();
		meshRendererSystem->SetMeshRenderer(test, AssetManager::Instance().GetAsset<RenderObject>(1));

		/*std::cout << "\STRESS TEST ECS\n====================================\n";
		srand(time(NULL));
		for (int i{}; i < 2500; ++i)
		{
			Entity ent{ ECSManager::Instance().CreateEntity() };
			ent->AddComponent<MeshRenderer>();
			transformSystem->SetPosition(ent, glm::vec3(0.f, 0.f, 25.f));
			transformSystem->SetScale(ent, glm::vec3(rand() % 10, rand() % 10, rand() % 10));
			meshRendererSystem->SetMeshRenderer(ent, vase);
		}*/

		Entity test3 = ECSManager::Instance().CreateEntity();
		transformSystem->SetPosition(test3, glm::vec3(0.f, 0.f, 0.f));

		Entity cam = ECSManager::Instance().CreateEntity();
		cam->GetComponent<Properties>().m_Name = "cam";
		cam->AddComponent<Camera>();


		//Entity audio = ECSManager::Instance().CreateEntity();
		//audio->AddComponent<Audio>();

		cameraSystem->SetIsMainCamera(cam, true);
		//ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetFocalPoint(cam, test->GetComponent<Transform>().m_Position);
		// _system_manager->GetSystem<PhysicsSystem>()->ConstructSphereCollider(test2, { 4, 10, 4 }, 2);
		//ECSSystemManager::Instance().GetSystem<AudioSystem>()->CompileAudio(audio);

		//ECSManager::Instance().SaveEntities("Demo.json");

		//SceneManager::Instance().SaveSceneAs("DemoScene");

		/*SceneManager::Instance().LoadScene("DemoScene");
		Entity cam2 = ECSManager::Instance().CreateEntity();
		cam2->GetComponent<Properties>().m_Name = "cam2";
		cam2->AddComponent<Camera>();
		cameraSystem->SetIsMainCamera(cam2, true);*/
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
		// Load set folders and names
		FileSystem::Instance().GenerateFolderFileNamesFile("FolderFileNames");

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
		ECSSystemManager::Instance().RegisterSystem<ParentingSystem>();
		ECSSystemManager::Instance().RegisterSystem<TransformSystem>();
		ECSSystemManager::Instance().RegisterSystem<PhysicsSystem>();
		ECSSystemManager::Instance().RegisterSystem<CameraSystem>();
		ECSSystemManager::Instance().RegisterSystem<AudioSystem>();
		ECSSystemManager::Instance().RegisterSystem<MeshRendererSystem>();

		// Allocate Default Size for Memory Manager
		MemoryManager::Instance().AllocateEntitySize(MemoryManager::Instance().GetConfigSize());
	}

	void Engine::Update()
	{
		// To remove eventually
		//ECSManager::Instance().TESTRUN();
		//AHHH();
		DemoScene();
		//ECSManager::Instance().STRESSTEST();

		while (!m_Window->ShouldWindowClose())
		{

			m_Window->BeginFrame();
			m_Window->UpdateDeltaTime();

			//Update
			Profiler::Instance().StartTimer("Update");
			ECSSystemManager::Instance().UpdateSystem();
			Profiler::Instance().EndTimer("Update");

			Profiler::Instance().StartTimer("OnDestroyEntities");
			ECSSystemManager::Instance().OnDestroyEntities();
			Profiler::Instance().EndTimer("OnDestroyEntities");

			Profiler::Instance().StartTimer("DeleteRemovalEntities");
			ECSManager::Instance().DeleteRemovalEntities();
			Profiler::Instance().EndTimer("DeleteRemovalEntities");

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