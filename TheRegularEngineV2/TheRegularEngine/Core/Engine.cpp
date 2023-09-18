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
#include <time.h>       /* time */
#include "Graphics/VulkanTexture.h"
#include "Assets/AssetManager.h"
#include "Graphics/ShaderCompiler.h"

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

	void DemoDeserialize()
	{
		auto vertHandle = 3;
		auto fragHandle = 4;

		std::unique_ptr<Shader>vert = ShaderCompiler::CompileShader("Resources/Shaders/Template.vert");
		vert->SetHandle(vertHandle);
		AssetManager::Instance().AddAsset(std::move(vert));

		std::unique_ptr<Shader> frag = ShaderCompiler::CompileShader("Resources/Shaders/Template.frag");
		frag->SetHandle(fragHandle);
		AssetManager::Instance().AddAsset(std::move(frag));

		SceneManager::Instance().LoadScene("DemoScene");
	}

	void DemoScene()
	{
		auto textureHandle = Asset::GetGUIDFromHex("140ecd34766a2024");
		auto textureHandle2 = Asset::GetGUIDFromHex("500a79fe5030f521");
		auto geomHandle = Asset::GetGUIDFromHex("d07c9c27d46df02f");
		auto vertHandle = 3;
		auto fragHandle = 4;
		auto DebugDrawVertHandle = 7;
		auto DebugDrawFragHandle = 8;
		auto matHandle = Asset::GetGUIDFromHex("74b283e6a2bed9d8");
		auto matHandle2 = Asset::GetGUIDFromHex("89f11168a1b5734c");

		Texture::RunCompiler("../Assets/140ecd34766a2024.desc");
		std::unique_ptr<VulkanTexture> vkt1 = std::make_unique<VulkanTexture>("../Assets/140ecd34766a2024.DDS");
		vkt1->SetHandle(1445318155641167908);
		AssetManager::Instance().AddAsset(std::move(vkt1));

		Texture::RunCompiler("../Assets/500a79fe5030f521.desc");
		std::unique_ptr<VulkanTexture> vkt2 = std::make_unique<VulkanTexture>("../Assets/500a79fe5030f521.DDS");
		vkt2->SetHandle(textureHandle2);
		AssetManager::Instance().AddAsset(std::move(vkt2));

		Geom::RunCompiler("../Assets/d07c9c27d46df02f.desc");
		std::unique_ptr<RenderObject> ro = std::make_unique<RenderObject>("../Assets/d07c9c27d46df02f.geom");
		ro->SetHandle(geomHandle);
		AssetManager::Instance().AddAsset(std::move(ro));

		std::unique_ptr<Shader>vert = ShaderCompiler::CompileShader("Resources/Shaders/Template.vert");
		vert->SetHandle(vertHandle);
		AssetManager::Instance().AddAsset(std::move(vert));

		std::unique_ptr<Shader> frag = ShaderCompiler::CompileShader("Resources/Shaders/Template.frag");
		frag->SetHandle(fragHandle);
		AssetManager::Instance().AddAsset(std::move(frag));

		//DebugDrawShaders
		std::unique_ptr<Shader> DebugDrawVert = ShaderCompiler::CompileShader("Resources/Shaders/DebugDrawLine.vert");
		DebugDrawVert->SetHandle(DebugDrawVertHandle);
		AssetManager::Instance().AddAsset(std::move(DebugDrawVert));

		std::unique_ptr<Shader> DebugDrawFrag = ShaderCompiler::CompileShader("Resources/Shaders/DebugDrawLine.frag");
		DebugDrawFrag->SetHandle(DebugDrawFragHandle);
		AssetManager::Instance().AddAsset(std::move(DebugDrawFrag));

		auto DebugVertShader = AssetManager::Instance().GetAsset<Shader>(DebugDrawVertHandle);
		auto DebugFragShader = AssetManager::Instance().GetAsset<Shader>(DebugDrawFragHandle);

		// Create a material instance
		auto VertShader = AssetManager::Instance().GetAsset<Shader>(vertHandle);
		auto FragShader = AssetManager::Instance().GetAsset<Shader>(fragHandle);
		std::unique_ptr<Material> mat1 = std::make_unique<Material>(VertShader, FragShader);
		mat1->SetHandle(matHandle);
		mat1->SetTextures(AssetManager::Instance().GetAsset<VulkanTexture>(textureHandle));
		mat1->SetTextures(AssetManager::Instance().GetAsset<VulkanTexture>(textureHandle2));
		AssetManager::Instance().AddAsset(std::move(mat1));

		std::unique_ptr<Material> mat2 = std::make_unique<Material>(VertShader, FragShader);
		mat2->SetHandle(matHandle2);
		mat2->SetTextures(AssetManager::Instance().GetAsset<VulkanTexture>(textureHandle2));
		mat2->SetTextures(AssetManager::Instance().GetAsset<VulkanTexture>(textureHandle));
		AssetManager::Instance().AddAsset(std::move(mat2));

		auto transformSystem = ECSSystemManager::Instance().GetSystem<TransformSystem>();
		auto meshRendererSystem = ECSSystemManager::Instance().GetSystem<MeshRendererSystem>();
		auto cameraSystem = ECSSystemManager::Instance().GetSystem<CameraSystem>();

		Entity test2 = ECSManager::Instance().CreateEntity();
		test2->GetComponent<Properties>().m_Name = "Test2";
		transformSystem->SetPosition(test2, glm::vec3(30.f, 10.f, 100.f));
		transformSystem->SetScale(test2, glm::vec3(5.f, 5.f, 5.f));
		transformSystem->SetRotation(test2, glm::vec3(0.f, 0.f, 45.f));
		test2->AddComponent<MeshRenderer>();
		meshRendererSystem->SetMeshRenderer(test2, AssetManager::Instance().GetAsset<RenderObject>(geomHandle));
		meshRendererSystem->SetMaterial(test2, AssetManager::Instance().GetAsset<Material>(matHandle));

		Entity test = ECSManager::Instance().CreateEntity();
		test->GetComponent<Properties>().m_Name = "Test";
		transformSystem->SetPosition(test, glm::vec3(0.f,0.f, 25.f));
		transformSystem->SetScale(test, glm::vec3(5.f, 5.f, 5.f));
		test->AddComponent<MeshRenderer>();
		meshRendererSystem->SetMeshRenderer(test, AssetManager::Instance().GetAsset<RenderObject>(geomHandle));
		meshRendererSystem->SetMaterial(test, AssetManager::Instance().GetAsset<Material>(matHandle2));

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

		/*Entity test3 = ECSManager::Instance().CreateEntity();
		transformSystem->SetPosition(test3, glm::vec3(0.f, 0.f, 0.f));*/

		Entity cam = ECSManager::Instance().CreateEntity();
		cam->GetComponent<Properties>().m_Name = "cam";
		cam->AddComponent<Camera>();
		cameraSystem->SetIsMainCamera(cam, true);


		//Entity audio = ECSManager::Instance().CreateEntity();
		//audio->AddComponent<Audio>();

		//ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetFocalPoint(cam, test->GetComponent<Transform>().m_Position);
		// _system_manager->GetSystem<PhysicsSystem>()->ConstructSphereCollider(test2, { 4, 10, 4 }, 2);
		//ECSSystemManager::Instance().GetSystem<AudioSystem>()->CompileAudio(audio);

		SceneManager::Instance().SaveSceneAs("DemoScene");

		//std::cout << "Main Camera is " << ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera()->GetName() << "\n";


		//cameraSystem->SetIsMainCamera(cam, false);
		//SceneManager::Instance().NewScene();
		//SceneManager::Instance().LoadScene("DemoScene");

		/*Entity cam2 = ECSManager::Instance().CreateEntity();
		cam2->GetComponent<Properties>().m_Name = "cam2";
		cam2->AddComponent<Camera>();
		cameraSystem->SetIsMainCamera(cam2, true);*/

		//std::cout << "Deserialized Main Camera is " << ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera()->GetName() << "\n";
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

		RegisterECS();
		//DemoDeserialize();
		DemoScene();

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
		ECSManager::Instance().RegisterComponent<Undeployed>("Undeployed", true);		// ignore
		ECSManager::Instance().RegisterComponent<Removal>("Removal", true);				// ignore
		ECSManager::Instance().RegisterComponent<Parenting>("Parenting", true);			// serialized
		ECSManager::Instance().RegisterComponent<Properties>("Properties", true);		// serialized
		ECSManager::Instance().RegisterComponent<Transform>("Transform");				// serialized
		ECSManager::Instance().RegisterComponent<MeshRenderer>("Mesh Renderer");		// 
		ECSManager::Instance().RegisterComponent<Camera>("Camera");						// serialized
		ECSManager::Instance().RegisterComponent<SphereCollider>("SphereCollider");
		ECSManager::Instance().RegisterComponent<BoxCollider>("BoxCollider");
		ECSManager::Instance().RegisterComponent<Rigidbody>("Rigidbody");
		ECSManager::Instance().RegisterComponent<Audio>("Audio");
		ECSManager::Instance().RegisterComponent<FEL>("FEL");							// serialized

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
		//DemoScene();
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