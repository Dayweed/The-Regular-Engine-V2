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
#include "Graphics/Light.h"
#include "Graphics/MeshRenderer.h"
#include "Graphics/Camera.h"

//TO DELETE
#pragma region TO DELETE TEST
#include <time.h>       /* time */
#include "Graphics/VulkanTexture.h"
#include "Resource/ResourceManager.h"
#include "Graphics/ShaderReflection.h"
#include "TextureDescriptorFile.h"	
#include "Physics/PhysicsComponents.h"
#include "ShaderTypes/PBRShader.h"

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
		SceneManager::Instance().LoadScene("../Scenes/DemoScene.json");
	}

	void DemoScene()
	{
		//std::cout << Resource::GetGUIDHex(Resource::GenerateGUID()) << "\n";

		auto textureHandle = Resource::GetGUIDFromHex("474d70e35d64e711"); //diffuse
		auto textureHandle2 = Resource::GetGUIDFromHex("d3464713e4f44bee"); //normal
		auto textureHandle3 = Resource::GetGUIDFromHex("8a0c8bee2a64d76b"); //roughness
		auto textureHandle4 = Resource::GetGUIDFromHex("13392e8301ebb46"); //AO
		auto skullHandle = Resource::GetGUIDFromHex("b1d2057915001876"); //skull
		auto planeHandle = Resource::GetGUIDFromHex("b262c8535c88eff7"); //plane
		auto FinalPassShaderHandle = 4;
		auto matHandle = Resource::GetGUIDFromHex("74b283e6a2bed9d8");

#if 0
		auto AnimationHandle = 5;
		auto AnimationtextureHandle1 = Resource::GetGUIDFromHex("9c6509635ee2d750");
		auto AnimationtextureHandle2 = Resource::GetGUIDFromHex("52ba56f854e86f56");
		auto AnimationtextureHandle3 = Resource::GetGUIDFromHex("547865c1f61ef1f9");
		auto AnimationtextureHandle4 = Resource::GetGUIDFromHex("c076cd64a7491d7");
		auto AnimationtextureHandle5 = Resource::GetGUIDFromHex("6b2822ce3972f53");
#endif

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
#if 0
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

		//AnimationShaders
		std::unique_ptr<Shader> AnimationVert = ShaderCompiler::DeserializeReflectShader("../Resources/Animation.TREshader");
		AnimationVert->SetHandle(AnimationHandle);
		ResourceManager::Instance().AddResource(std::move(AnimationVert));
#endif

		//Geom::RunCompiler("../Assets/b1d2057915001876.desc");
		std::unique_ptr<RenderObject> ro = std::make_unique<RenderObject>("../Resources/b1d2057915001876.geom");
		ro->SetHandle(skullHandle);
		ResourceManager::Instance().AddResource(std::move(ro));

		//Geom::RunCompiler("../Assets/b262c8535c88eff7.desc");
		std::unique_ptr<RenderObject> plane = std::make_unique<RenderObject>("../Resources/b262c8535c88eff7.geom");
		plane->SetHandle(planeHandle);
		ResourceManager::Instance().AddResource(std::move(plane));

		//FinalPassShader
		std::unique_ptr<Shader> FinalPassShader = ShaderCompiler::DeserializeReflectShader("../Resources/CompositePass.TREshader");
		FinalPassShader->SetHandle(FinalPassShaderHandle);
		ResourceManager::Instance().AddResource(std::move(FinalPassShader));

		// Create a material instance
		auto VertShader = ResourceManager::Instance().GetResource<Shader>(PBR::GetShaderHandle());
		std::unique_ptr<Material> mat1 = std::make_unique<Material>(VertShader);
		mat1->SetHandle(matHandle);
		mat1->SetTexture("DiffuseMap", ResourceManager::Instance().GetResource<VulkanTexture>(textureHandle));
		mat1->SetTexture("NormalMap", ResourceManager::Instance().GetResource<VulkanTexture>(textureHandle2));
		mat1->SetTexture("RoughnessMap", ResourceManager::Instance().GetResource<VulkanTexture>(textureHandle3));
		mat1->SetTexture("AOMap", ResourceManager::Instance().GetResource<VulkanTexture>(textureHandle4));
		ResourceManager::Instance().AddResource(std::move(mat1));

		auto meshRendererSystem = ECSSystemManager::Instance().GetSystem<MeshRendererSystem>();
		auto cameraSystem = ECSSystemManager::Instance().GetSystem<CameraSystem>();
		auto audioSystem = ECSSystemManager::Instance().GetSystem<AudioSystem>();

		{
			Entity test = ECSManager::Instance().CreateEntity();
			test->GetComponent<Properties>().m_Name = "Test";

			Transform& testTransform{ test->GetComponent<Transform>() };
			testTransform.m_Position = glm::vec3(0.f, 20.f, 180.f);
			testTransform.m_Scale = glm::vec3(0.2f, 0.2f, 0.2f);
			testTransform.m_Rotation = glm::vec3(0, 180.f, 0);
			testTransform.m_IsDirty = true;

			test->AddComponent<MeshRenderer>();
			meshRendererSystem->SetMeshRenderer(test, ResourceManager::Instance().GetResource<RenderObject>(skullHandle));
			meshRendererSystem->SetMaterial(test, ResourceManager::Instance().GetResource<Material>(matHandle));

			test->AddComponent<Audio>();
			//audioSystem->SetFileName(test, "ViveLeFromageBGM1.wav");
			//audioSystem->SetLoop(test, true);
			//audioSystem->SetSpatialize(test,true);
			//audioSystem->CompileAudio(test);
			//audioSystem->SetSourceRadius(test, 50.f, 150.f);

			//test->AddComponent<SphereCollider>();
			//test->AddComponent<Rigidbody>();
		}

		{
			Entity test2 = ECSManager::Instance().CreateEntity();
			test2->GetComponent<Properties>().m_Name = "Test2";

			Transform& test2Transform{ test2->GetComponent<Transform>() };
			test2Transform.m_Position = glm::vec3(50.f, 20.f, 180.f);
			test2Transform.m_Scale = glm::vec3(0.2f, 0.2f, 0.2f);
			test2Transform.m_Rotation = glm::vec3(0, 180.f, 0);
			test2Transform.m_IsDirty = true;

			test2->AddComponent<MeshRenderer>();
			meshRendererSystem->SetMeshRenderer(test2, ResourceManager::Instance().GetResource<RenderObject>(skullHandle));
		}

		{
			Entity test3 = ECSManager::Instance().CreateEntity();
			test3->GetComponent<Properties>().m_Name = "Test3";

			Transform& test3Transform{ test3->GetComponent<Transform>() };
			test3Transform.m_Position = glm::vec3(-50.f, 20.f, 180.f);
			test3Transform.m_Scale = glm::vec3(0.2f, 0.2f, 0.2f);
			test3Transform.m_Rotation = glm::vec3(0, 180.f, 0);
			test3Transform.m_IsDirty = true;

			test3->AddComponent<MeshRenderer>();
			meshRendererSystem->SetMeshRenderer(test3, ResourceManager::Instance().GetResource<RenderObject>(skullHandle));
			meshRendererSystem->SetMaterial(test3, ResourceManager::Instance().GetResource<Material>(matHandle));
		}

		{
			Entity planeCollider = ECSManager::Instance().CreateEntity();
			planeCollider->GetComponent<Properties>().m_Name = "Plane collider";

			Transform& planeTransform{ planeCollider->GetComponent<Transform>() };
			planeTransform.m_Position = glm::vec3(0.f, -35.f, 100.f);
			planeTransform.m_Scale = glm::vec3(10.f, 10.f, 10.f);
			planeTransform.m_Rotation = glm::vec3(0, 0, 0);
			planeTransform.m_IsDirty = true;

			planeCollider->AddComponent<MeshRenderer>();
			meshRendererSystem->SetMeshRenderer(planeCollider, ResourceManager::Instance().GetResource<RenderObject>(planeHandle));
		}

		{
			Entity cam = ECSManager::Instance().CreateEntity();
			cam->GetComponent<Properties>().m_Name = "cam";
			cam->AddComponent<Camera>();
			cameraSystem->SetIsMainCamera(cam, true);

			cam->AddComponent<AudioListener>();
			audioSystem->SetListenerPosition(cam);
		}

		{
			Entity light = ECSManager::Instance().CreateEntity();
			light->GetComponent<Properties>().m_Name = "Direction Light";
			light->AddComponent<DirectionalLight>();
			light->GetComponent<Transform>().m_Rotation.x = 45.f;
			light->GetComponent<Transform>().m_IsDirty = true;
		}

		{
			//// Parent child prefabing test
			//Entity prefabParent = ECSManager::Instance().CreateEntity();
			//prefabParent->GetComponent<Properties>().m_Name = "prefabParent";
			//Transform& transform3 = prefabParent->GetComponent<Transform>();
			//transform3.m_Position = glm::vec3(0.f, 50.f, 100.f);
			//transform3.m_Scale = glm::vec3(0.2f, 0.2f, 0.2f);
			//transform3.m_Rotation = glm::vec3(0, 180.f, 0);
			//transform3.m_IsDirty = true;
			//prefabParent->AddComponent<MeshRenderer>();
			//meshRendererSystem->SetMeshRenderer(prefabParent, ResourceManager::Instance().GetResource<RenderObject>(skullHandle));
			//meshRendererSystem->SetMaterial(prefabParent, ResourceManager::Instance().GetResource<Material>(matHandle));

			//Entity prefabChild = ECSManager::Instance().CreateEntity();
			//prefabChild->GetComponent<Properties>().m_Name = "prefabChild";
			//Transform& transform4 = prefabChild->GetComponent<Transform>();
			//transform4.m_Position = glm::vec3(-100.f, 50.f, 100.f);
			//transform4.m_Scale = glm::vec3(0.1f, 0.1f, 0.1f);
			//transform4.m_Rotation = glm::vec3(0, 180.f, 0);
			//transform4.m_IsDirty = true;
			//prefabChild->AddComponent<MeshRenderer>();
			//prefabChild->AddComponent<FAKEFEL>();
			//meshRendererSystem->SetMeshRenderer(prefabChild, ResourceManager::Instance().GetResource<RenderObject>(skullHandle));
			//meshRendererSystem->SetMaterial(prefabChild, ResourceManager::Instance().GetResource<Material>(matHandle));

			//ECSSystemManager::Instance().GetSystem<ParentingSystem>()->AddChild(prefabParent, prefabChild);
		}

		{
			//dont delete this
			//testing hierarchy entities
			Entity grandparent1 = ECSManager::Instance().CreateEntity("grandparent1");
			Entity parent1 = ECSManager::Instance().CreateEntity("parent1");
			ECSSystemManager::Instance().GetSystem<ParentingSystem>()->SetParent(parent1, grandparent1);

			Entity parent2 = ECSManager::Instance().CreateEntity("parent2");
			ECSSystemManager::Instance().GetSystem<ParentingSystem>()->SetParent(parent2, grandparent1);

			Entity child1 = ECSManager::Instance().CreateEntity("child1");
			ECSSystemManager::Instance().GetSystem<ParentingSystem>()->SetParent(child1, parent1);
			Entity child2 = ECSManager::Instance().CreateEntity("child2");
			ECSSystemManager::Instance().GetSystem<ParentingSystem>()->SetParent(child2, parent1);

			Entity child3 = ECSManager::Instance().CreateEntity("child3");
			ECSSystemManager::Instance().GetSystem<ParentingSystem>()->SetParent(child3, parent2);
			Entity child4 = ECSManager::Instance().CreateEntity("child4");
			ECSSystemManager::Instance().GetSystem<ParentingSystem>()->SetParent(child4, parent2);


			Entity grandparent2 = ECSManager::Instance().CreateEntity("grandparent2");
			Entity parent3 = ECSManager::Instance().CreateEntity("parent3");
			ECSSystemManager::Instance().GetSystem<ParentingSystem>()->SetParent(parent3, grandparent2);

			Entity parent4 = ECSManager::Instance().CreateEntity("parent4");
			ECSSystemManager::Instance().GetSystem<ParentingSystem>()->SetParent(parent4, grandparent2);

			Entity child5 = ECSManager::Instance().CreateEntity("child5");
			ECSSystemManager::Instance().GetSystem<ParentingSystem>()->SetParent(child5, parent3);
			Entity child6 = ECSManager::Instance().CreateEntity("child6");
			ECSSystemManager::Instance().GetSystem<ParentingSystem>()->SetParent(child6, parent3);

			Entity child7 = ECSManager::Instance().CreateEntity("child7");
			ECSSystemManager::Instance().GetSystem<ParentingSystem>()->SetParent(child7, parent4);
			Entity child8 = ECSManager::Instance().CreateEntity("child8");
			ECSSystemManager::Instance().GetSystem<ParentingSystem>()->SetParent(child8, parent4);
		}

		//SceneManager::Instance().SaveSceneAs("../Scenes/DemoScene.json");
	}
}
#pragma endregion TO DELETE TEST

namespace TRE
{
	Engine* Engine::s_Instance = nullptr;

	const std::shared_ptr<SceneRenderer>& Engine::GetMainSceneRenderer()
	{
		return m_SceneRenderer;
	}

	const std::shared_ptr<Window>& Engine::GetWindow()
	{
		return m_Window;
	}

	const EngineInfo& Engine::GetEngineInfo()
	{
		return m_EngineInfo;
	}

	Engine& Engine::GetInstance()
	{
		return *s_Instance;
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
		Shader::SetupShaders();

		//DemoDeserialize();
		DemoScene();

		m_SceneRenderer = std::make_shared<SceneRenderer>(m_Window->GetRenderContext()->GetDeviceInternally());
		Renderer::Init();
		m_SceneRenderer->Initialize();

		if (m_EngineInfo.MaximizeWindow)
		{
			m_Window->MaximizeWindow();
		}

		if (m_EngineInfo.EnableEditor)
			m_VulkanEditor = std::make_shared<VulkanEditor>(m_Window->GetRenderContext()->GetDeviceInternally());

		ScriptEngine::InitMono();
		ScriptEngine::BindFunctions();
		//ScriptEngine::TestScriptingEngine();
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
		ECSManager::Instance().RegisterComponent<Undeployed>("Undeployed", true, false); // ignore, ignore
		ECSManager::Instance().RegisterComponent<Removal>("Removal", true, false);       // ignore, ignore
		ECSManager::Instance().RegisterComponent<Prefabing>("Prefabing", true, false);   // ignore, ignore
		ECSManager::Instance().RegisterComponent<Parenting>("Parenting", true, false);   // serialized, reflected
		ECSManager::Instance().RegisterComponent<Properties>("Properties", true, false); // serialized, reflected
		ECSManager::Instance().RegisterComponent<Transform>("Transform", false, false);  // serialized, reflected
		ECSManager::Instance().RegisterComponent<MeshRenderer>("Mesh Renderer");         // 
		ECSManager::Instance().RegisterComponent<Camera>("Camera");                      // serialized, reflected
		ECSManager::Instance().RegisterComponent<Rigidbody>("Rigidbody");                // serialized, reflected
		ECSManager::Instance().RegisterComponent<SphereCollider>("SphereCollider");      // serialized, reflected
		ECSManager::Instance().RegisterComponent<BoxCollider>("BoxCollider");            // serialized, reflected
		ECSManager::Instance().RegisterComponent<CapsuleCollider>("CapsuleCollider");    // serialized, reflected
		ECSManager::Instance().RegisterComponent<Audio>("Audio");                        // 
		ECSManager::Instance().RegisterComponent<AudioListener>("AudioListener");        // 
		ECSManager::Instance().RegisterComponent<FEL>("FEL");                            // serialized
		ECSManager::Instance().RegisterComponent<FAKEFEL>("FAKEFEL");                    // serialized, reflected
		ECSManager::Instance().RegisterComponent<DirectionalLight>("Directional Light"); // 

		// Register Systems
		ECSSystemManager::Instance().RegisterSystem<PrefabSystem>();
		ECSSystemManager::Instance().RegisterSystem<ParentingSystem>();
		ECSSystemManager::Instance().RegisterSystem<PhysicsSystem>();
		ECSSystemManager::Instance().RegisterSystem<CameraSystem>();
		ECSSystemManager::Instance().RegisterSystem<AudioSystem>();
		ECSSystemManager::Instance().RegisterSystem<MeshRendererSystem>();
		ECSSystemManager::Instance().RegisterSystem<TransformSystem>();
		ECSSystemManager::Instance().RegisterSystem<LightSystem>();

		// Allocate Default Size for Memory Manager
		MemoryManager::Instance().AllocateEntitySize(MemoryManager::Instance().GetConfigSize());
	}

	void Engine::Update()
	{
		while (!m_Window->ShouldWindowClose() && m_Running)
		{
			m_Window->UpdateDeltaTime();

			m_Window->BeginFrame();
			m_SceneRenderer->BeginFrame();

			// Update
			Profiler::Instance().StartTimer("UpdateSystem");
			ECSSystemManager::Instance().UpdateSystem();
			Profiler::Instance().EndTimer("UpdateSystem");

			// Game Running Update
			if (GameLoop::Instance().IsGameRunning())
			{
				Profiler::Instance().StartTimer("GameUpdateSystem");
				ECSSystemManager::Instance().GameUpdateSystem();
				Profiler::Instance().EndTimer("GameUpdateSystem");
			}

			// Late Update
			Profiler::Instance().StartTimer("LateUpdateSystem");
			ECSSystemManager::Instance().LateUpdateSystem();
			Profiler::Instance().EndTimer("LateUpdateSystem");

			// OnReset Scene
			if (GameLoop::Instance().GetSceneReset())
			{
				Profiler::Instance().StartTimer("BeforeReset");
				ECSSystemManager::Instance().BeforeReset();
				Profiler::Instance().EndTimer("BeforeReset");

				// Copy registry and components
				ECSManager::Instance().CopyRegistry(GameLoop::Instance().GetBackUpRegistry());
				// Clear Backup
				GameLoop::Instance().GetBackUpRegistry().clear();

				Profiler::Instance().StartTimer("OnReset");
				ECSSystemManager::Instance().OnReset();
				Profiler::Instance().EndTimer("OnReset");

				GameLoop::Instance().SetSceneReset(false);
			}

			Profiler::Instance().StartTimer("OnDestroyEntities");
			ECSSystemManager::Instance().OnDestroyEntities();
			Profiler::Instance().EndTimer("OnDestroyEntities");

			Profiler::Instance().StartTimer("DeleteRemovalEntities");
			ECSManager::Instance().DeleteRemovalEntities();
			Profiler::Instance().EndTimer("DeleteRemovalEntities");

			m_SceneRenderer->EndFrame();

			// Imgui Update (Editor Draw and Update Inspector, Always 1 Frame delayed)
			if (m_EngineInfo.EnableEditor)
			{
				Profiler::Instance().StartTimer("Imgui");
				m_VulkanEditor->BeginFrame();
				EditorSystemManager::Instance().UpdateSystem();
				m_VulkanEditor->EndFrame();
				Profiler::Instance().EndTimer("Imgui");
			}
			else //Renders straight to swapchain
			{
				Renderer::RenderToSwapChain();
			}

			//Draw
			Profiler::Instance().StartTimer("Draw");
			m_Window->SwapBuffers();
			m_Window->PollEvents();
			Profiler::Instance().EndTimer("Draw");

			if (ScriptEngine::CreatedScriptObject == true)
			{
				ScriptEngine::UpdateScriptingEngine();
			}
			
			// THIS IS COMMENTED OUT UNTIL IMGUI IS UP, iteration 1 would be used for displaying until IMGUI can use iteration 2
			Profiler::Instance().PrintTimers();
		}
		
	}

	void Engine::Shutdown()
	{
		m_Running = false;
		ResourceManager::Instance().DestroyAllResources();
		EntityCopier::Instance().Shutdown();
		GameLoop::Instance().Shutdown();
		ECSManager::Instance().DestroyAll();
		ECSSystemManager::Instance().ShutdownSystem();
		EditorSystemManager::Instance().ShutdownSystem();
		MemoryManager::Instance().DeleteEntities();
		Renderer::Shutdown();
	}
}