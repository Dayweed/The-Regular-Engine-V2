#include "pch.h"
#include "TREIncludes.h"
#include "Engine.h"
#include "GameLoop.h"
#include "ECS.h"
#include "Transform.h"
#include "SceneManager.h"
#include "MemoryManager.h"
#include "Profiler.h"
#include "Physics/PhysicsSystem.h"
#include "Audio/AudioSystem.h"
#include "Logger.h"
#include "Scripting/ScriptEngine.h"

//TO DELETE
#pragma region TO DELETE TEST
#include "Graphics/MeshRenderer.h"
#include "Graphics/Camera.h"
#include <time.h>       /* time */
#include "Graphics/VulkanTexture.h"
#include "Resource/ResourceManager.h"
#include "Graphics/ShaderCompiler.h"
#include "TextureDescriptorFile.h"	
#include "Physics/PhysicsComponents.h"

#include "Demo/Demo.h"

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
		std::string fileName{ "../Scenes/AHHHScene.json" };
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
		auto DebugDrawVertHandle = 7;
		auto DebugDrawFragHandle = 8;

		std::unique_ptr<Shader>vert = ShaderCompiler::CompileShader("Resources/Shaders/PBR.vert");
		vert->SetHandle(vertHandle);
		ResourceManager::Instance().AddResource(std::move(vert));

		std::unique_ptr<Shader> frag = ShaderCompiler::CompileShader("Resources/Shaders/PBR.frag");
		frag->SetHandle(fragHandle);
		ResourceManager::Instance().AddResource(std::move(frag));

		//DebugDrawShaders
		std::unique_ptr<Shader> DebugDrawVert = ShaderCompiler::CompileShader("Resources/Shaders/DebugDrawLine.vert");
		DebugDrawVert->SetHandle(DebugDrawVertHandle);
		ResourceManager::Instance().AddResource(std::move(DebugDrawVert));

		std::unique_ptr<Shader> DebugDrawFrag = ShaderCompiler::CompileShader("Resources/Shaders/DebugDrawLine.frag");
		DebugDrawFrag->SetHandle(DebugDrawFragHandle);
		ResourceManager::Instance().AddResource(std::move(DebugDrawFrag));

		SceneManager::Instance().LoadScene("../Scenes/DemoScene.json");
	}

	void DemoScene()
	{
		//std::cout << Resource::GetGUIDHex(Resource::GenerateGUID()) << "\n";

		auto textureHandle = Resource::GetGUIDFromHex("474d70e35d64e711"); //diffuse
		auto textureHandle2 = Resource::GetGUIDFromHex("d3464713e4f44bee"); //normal
		auto textureHandle3 = Resource::GetGUIDFromHex("8a0c8bee2a64d76b"); //roughness
		auto textureHandle4 = Resource::GetGUIDFromHex("13392e8301ebb46"); //AO
		auto skullHandle = Resource::GetGUIDFromHex("b1d2057915001876");
		auto vertHandle = 3;
		auto fragHandle = 4;
		auto AnimationVertHandle = 5;
		auto AnimationFragHandle = 6;
		auto DebugDrawVertHandle = 7;
		auto DebugDrawFragHandle = 8;
		auto matHandle = Resource::GetGUIDFromHex("74b283e6a2bed9d8");
		auto matHandle2 = Resource::GetGUIDFromHex("89f11168a1b5734c");

		auto AnimationtextureHandle1 = Resource::GetGUIDFromHex("9c6509635ee2d750");
		auto AnimationtextureHandle2 = Resource::GetGUIDFromHex("52ba56f854e86f56");
		auto AnimationtextureHandle3 = Resource::GetGUIDFromHex("547865c1f61ef1f9");
		auto AnimationtextureHandle4 = Resource::GetGUIDFromHex("c076cd64a7491d7");
		auto AnimationtextureHandle5 = Resource::GetGUIDFromHex("6b2822ce3972f53");

		//Texture::RunCompiler("../Assets/474d70e35d64e711.desc");
		std::unique_ptr<VulkanTexture> vkt1 = std::make_unique<VulkanTexture>("../Resources/474d70e35d64e711.DDS");
		vkt1->SetHandle(textureHandle);
		ResourceManager::Instance().AddResource(std::move(vkt1));

		//Texture::RunCompiler("../Assets/d3464713e4f44bee.desc");
		std::unique_ptr<VulkanTexture> vkt2 = std::make_unique<VulkanTexture>("../Resources/d3464713e4f44bee.DDS");
		vkt2->SetHandle(textureHandle2);
		ResourceManager::Instance().AddResource(std::move(vkt2));

		//Texture::RunCompiler("../Assets/8a0c8bee2a64d76b.desc");
		std::unique_ptr<VulkanTexture> vkt3 = std::make_unique<VulkanTexture>("../Resources/8a0c8bee2a64d76b.DDS");
		vkt3->SetHandle(textureHandle3);
		ResourceManager::Instance().AddResource(std::move(vkt3));

		//Texture::RunCompiler("../Assets/13392e8301ebb46.desc");
		std::unique_ptr<VulkanTexture> vkt4 = std::make_unique<VulkanTexture>("../Resources/13392e8301ebb46.DDS");
		vkt4->SetHandle(textureHandle4);
		ResourceManager::Instance().AddResource(std::move(vkt4));


		//Animation Textures//
		//Texture::RunCompiler("../Assets/9c6509635ee2d750.desc");
		std::unique_ptr<VulkanTexture> vkt5 = std::make_unique<VulkanTexture>("../Resources/9c6509635ee2d750.DDS");
		vkt5->SetHandle(AnimationtextureHandle1);
		ResourceManager::Instance().AddResource(std::move(vkt5));

		//Texture::RunCompiler("../Assets/52ba56f854e86f56.desc");
		std::unique_ptr<VulkanTexture> vkt6 = std::make_unique<VulkanTexture>("../Resources/52ba56f854e86f56.DDS");
		vkt6->SetHandle(AnimationtextureHandle2);
		ResourceManager::Instance().AddResource(std::move(vkt6));

		//Texture::RunCompiler("../Assets/547865c1f61ef1f9.desc");
		std::unique_ptr<VulkanTexture> vkt7 = std::make_unique<VulkanTexture>("../Resources/547865c1f61ef1f9.DDS");
		vkt7->SetHandle(AnimationtextureHandle3);
		ResourceManager::Instance().AddResource(std::move(vkt7));

		//Texture::RunCompiler("../Assets/c076cd64a7491d7.desc");
		std::unique_ptr<VulkanTexture> vkt8 = std::make_unique<VulkanTexture>("../Resources/c076cd64a7491d7.DDS");
		vkt8->SetHandle(AnimationtextureHandle4);
		ResourceManager::Instance().AddResource(std::move(vkt8));
		
		//Texture::RunCompiler("../Assets/6b2822ce3972f53.desc");
		std::unique_ptr<VulkanTexture> vkt9 = std::make_unique<VulkanTexture>("../Resources/6b2822ce3972f53.DDS");
		vkt9->SetHandle(AnimationtextureHandle5);
		ResourceManager::Instance().AddResource(std::move(vkt9));
		//Animation Textures//

		//Geom::RunCompiler("../Assets/b1d2057915001876.desc");
		std::unique_ptr<RenderObject> ro = std::make_unique<RenderObject>("../Resources/b1d2057915001876.geom");
		ro->SetHandle(skullHandle);
		ResourceManager::Instance().AddResource(std::move(ro));

		std::unique_ptr<Shader>vert = ShaderCompiler::CompileShader("../Resources/Shaders/PBR.vert");
		vert->SetHandle(vertHandle);
		ResourceManager::Instance().AddResource(std::move(vert));

		std::unique_ptr<Shader> frag = ShaderCompiler::CompileShader("../Resources/Shaders/PBR.frag");
		frag->SetHandle(fragHandle);
		ResourceManager::Instance().AddResource(std::move(frag));

		//DebugDrawShaders
		std::unique_ptr<Shader> DebugDrawVert = ShaderCompiler::CompileShader("../Resources/Shaders/DebugDrawLine.vert");
		DebugDrawVert->SetHandle(DebugDrawVertHandle);
		ResourceManager::Instance().AddResource(std::move(DebugDrawVert));

		std::unique_ptr<Shader> DebugDrawFrag = ShaderCompiler::CompileShader("../Resources/Shaders/DebugDrawLine.frag");
		DebugDrawFrag->SetHandle(DebugDrawFragHandle);
		ResourceManager::Instance().AddResource(std::move(DebugDrawFrag));

		auto DebugVertShader = ResourceManager::Instance().GetResource<Shader>(DebugDrawVertHandle);
		auto DebugFragShader = ResourceManager::Instance().GetResource<Shader>(DebugDrawFragHandle);

		//AnimationShaders
		std::unique_ptr<Shader> AnimationVert = ShaderCompiler::CompileShader("../Resources/Shaders/Animation.vert");
		AnimationVert->SetHandle(AnimationVertHandle);
		ResourceManager::Instance().AddResource(std::move(AnimationVert));

		std::unique_ptr<Shader> AnimationFrag = ShaderCompiler::CompileShader("../Resources/Shaders/Animation.frag");
		AnimationFrag->SetHandle(AnimationFragHandle);
		ResourceManager::Instance().AddResource(std::move(AnimationFrag));

		auto AnimationVertShader = ResourceManager::Instance().GetResource<Shader>(AnimationVertHandle);
		auto AnimationFragShader = ResourceManager::Instance().GetResource<Shader>(AnimationFragHandle);

		// Create a material instance
		auto VertShader = ResourceManager::Instance().GetResource<Shader>(vertHandle);
		auto FragShader = ResourceManager::Instance().GetResource<Shader>(fragHandle);
		std::unique_ptr<Material> mat1 = std::make_unique<Material>(VertShader, FragShader);
		mat1->SetHandle(matHandle);
		mat1->SetTextures(ResourceManager::Instance().GetResource<VulkanTexture>(textureHandle));
		mat1->SetTextures(ResourceManager::Instance().GetResource<VulkanTexture>(textureHandle2));
		mat1->SetTextures(ResourceManager::Instance().GetResource<VulkanTexture>(textureHandle3));
		mat1->SetTextures(ResourceManager::Instance().GetResource<VulkanTexture>(textureHandle4));
		ResourceManager::Instance().AddResource(std::move(mat1));

	/*	std::unique_ptr<Material> mat2 = std::make_unique<Material>(VertShader, FragShader);
		mat2->SetHandle(matHandle2);
		mat2->SetTextures(ResourceManager::Instance().GetResource<VulkanTexture>(textureHandle));
		mat2->SetTextures(ResourceManager::Instance().GetResource<VulkanTexture>(textureHandle2));
		mat2->SetTextures(ResourceManager::Instance().GetResource<VulkanTexture>(textureHandle3));
		mat2->SetTextures(ResourceManager::Instance().GetResource<VulkanTexture>(textureHandle4));
		ResourceManager::Instance().AddResource(std::move(mat2));*/

		auto transformSystem = ECSSystemManager::Instance().GetSystem<TransformSystem>();
		auto meshRendererSystem = ECSSystemManager::Instance().GetSystem<MeshRendererSystem>();
		auto cameraSystem = ECSSystemManager::Instance().GetSystem<CameraSystem>();
		auto audioSystem = ECSSystemManager::Instance().GetSystem<AudioSystem>();

		Entity test = ECSManager::Instance().CreateEntity();
		test->GetComponent<Properties>().m_Name = "Test";
		transformSystem->SetPosition(test, glm::vec3(0.f, 20.f, 180.f));
		transformSystem->SetScale(test, glm::vec3(0.2f, 0.2f, 0.2f));
		transformSystem->SetRotation(test, glm::vec3(0,180.f,0));
		test->AddComponent<MeshRenderer>();
		meshRendererSystem->SetMeshRenderer(test, ResourceManager::Instance().GetResource<RenderObject>(skullHandle));
		meshRendererSystem->SetMaterial(test, ResourceManager::Instance().GetResource<Material>(matHandle));
		test->AddComponent<Audio>();
		audioSystem->SetFileName(test, "ViveLeFromageBGM1.wav");
		audioSystem->SetLoop(test, true);
		audioSystem->SetSpatialize(test,true);
		audioSystem->CompileAudio(test);
		audioSystem->SetSourceRadius(test, 50.f, 150.f);
		test->AddComponent<SphereCollider>();
		test->AddComponent<Rigidbody>();

		/*Entity test2 = ECSManager::Instance().CreateEntity();
		test2->GetComponent<Properties>().m_Name = "Test2";
		transformSystem->SetPosition(test2, glm::vec3(0.f, 20.f, 180.f));
		transformSystem->SetScale(test2, glm::vec3(0.5f, 0.5f, 0.5f));
		transformSystem->SetRotation(test2, glm::vec3(0, 180.f, 0));
		test2->AddComponent<MeshRenderer>();
		meshRendererSystem->SetMeshRenderer(test2, ResourceManager::Instance().GetResource<RenderObject>(skullHandle));
		meshRendererSystem->SetMaterial(test2, ResourceManager::Instance().GetResource<Material>(matHandle2));*/

		Entity cam = ECSManager::Instance().CreateEntity();
		cam->GetComponent<Properties>().m_Name = "cam";
		cam->AddComponent<Camera>();
		cameraSystem->SetIsMainCamera(cam, true);
		cam->AddComponent<AudioListener>();
		audioSystem->SetListenerPosition(cam);

		//Entity audio = ECSManager::Instance().CreateEntity();
		//audio->AddComponent<Audio>();

		//ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetFocalPoint(cam, test->GetComponent<Transform>().m_Position);
		// _system_manager->GetSystem<PhysicsSystem>()->ConstructSphereCollider(test2, { 4, 10, 4 }, 2);
		//ECSSystemManager::Instance().GetSystem<AudioSystem>()->CompileAudio(audio);

		//SceneManager::Instance().SaveSceneAs("../Scenes/DemoScene.json");

		//SceneManager::Instance().NewScene();
		//SceneManager::Instance().LoadScene("../Scenes/DemoScene.json");
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

		GameLoop::Instance().Init();
		RegisterECS();
		//DemoDeserialize();
		DemoScene();

		ScriptEngine::InitMono();
		ScriptEngine::BindFunctions();
		//ScriptEngine::TestScriptingEngine();

		m_Renderer = std::make_shared<Renderer>(m_Window->GetRenderContext()->GetDeviceInternally());
		m_Renderer->Initialize();

		if (m_EngineInfo.EnableEditor)
			m_VulkanEditor = std::make_shared<VulkanEditor>(m_Window->GetRenderContext()->GetDeviceInternally());
	}

	Engine::~Engine()
	{
		Shutdown();
	}

	void Engine::RegisterECS()
	{
		// Load set folders and names
		FileSystem::Instance().GenerateFolderFileNamesFile("FolderFileNames");

		// Register Components
		ECSManager::Instance().RegisterComponent<Undeployed>("Undeployed", true, false);		// ignore, ignore
		ECSManager::Instance().RegisterComponent<Removal>("Removal", true, false);			// ignore, ignore
		ECSManager::Instance().RegisterComponent<Prefabing>("Prefabing", true, false);		// ignore, ignore
		ECSManager::Instance().RegisterComponent<Parenting>("Parenting", true, false);		// serialized, reflected
		ECSManager::Instance().RegisterComponent<Properties>("Properties", true, false);		// serialized, reflected
		ECSManager::Instance().RegisterComponent<Transform>("Transform", false, false);		// serialized, reflected
		ECSManager::Instance().RegisterComponent<MeshRenderer>("Mesh Renderer");							// 
		ECSManager::Instance().RegisterComponent<Camera>("Camera");											// serialized, reflected
		ECSManager::Instance().RegisterComponent<SphereCollider>("SphereCollider");							// reflected
		ECSManager::Instance().RegisterComponent<BoxCollider>("BoxCollider");								// reflected
		ECSManager::Instance().RegisterComponent<Rigidbody>("Rigidbody");									// reflected
		ECSManager::Instance().RegisterComponent<Audio>("Audio");											// 
		ECSManager::Instance().RegisterComponent<AudioListener>("AudioListener");							// 
		ECSManager::Instance().RegisterComponent<FEL>("FEL");												// serialized
		ECSManager::Instance().RegisterComponent<FAKEFEL>("FAKEFEL");										// serialized, reflected

		// Register Systems
		ECSSystemManager::Instance().RegisterSystem<PrefabSystem>();
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
		while (!m_Window->ShouldWindowClose() && m_Running)
		{
			m_Window->UpdateDeltaTime();

			//Update
			if (GameLoop::Instance().IsGameRunning())
			{
				Profiler::Instance().StartTimer("Update");
				ECSSystemManager::Instance().UpdateSystem();
				Profiler::Instance().EndTimer("Update");
			}

			Profiler::Instance().StartTimer("OnDestroyEntities");
			ECSSystemManager::Instance().OnDestroyEntities();
			Profiler::Instance().EndTimer("OnDestroyEntities");

			Profiler::Instance().StartTimer("DeleteRemovalEntities");
			ECSManager::Instance().DeleteRemovalEntities();
			Profiler::Instance().EndTimer("DeleteRemovalEntities");

			m_Window->BeginFrame();
			m_Renderer->BeginFrame();
			m_Renderer->EndFrame();

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
			Profiler::Instance().PrintTimers();
		}
	}

	void Engine::Shutdown()
	{
		m_Running = false;
		ECSManager::Instance().DestroyAll();
		ECSSystemManager::Instance().ShutdownSystem();
		EditorSystemManager::Instance().ShutdownSystem();
		MemoryManager::Instance().DeleteEntities();
		GameLoop::Instance().Shutdown();
	}
}