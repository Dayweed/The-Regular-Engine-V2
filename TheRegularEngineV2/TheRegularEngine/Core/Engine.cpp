#include "pch.h"
#include "TREIncludes.h"
#include "Engine.h"
#include "GameLoop.h"
#include "ECS.h"
#include "SceneManager.h"
#include "MemoryManager.h"
#include "Profiler.h"
#include "Physics/PhysicsSystem.h"
#include "Audio/AudioSystem.h"
#include "Scripting/ScriptingSystem.h"
#include "Scripting/ScriptEngine.h"
#include "Graphics/Camera.h"
#include "Graphics/EditorCamera.h"
#include "SceneManager.h"
#include "Graphics/AnimationSystem.h"
#include "InputHandler/InputHandler.h"

namespace TRE
{
	Engine* Engine::s_Instance = nullptr;

	const std::shared_ptr<SceneRenderer>& Engine::GetMainSceneRenderer()
	{
		return m_SceneRenderer;
	}

	const std::shared_ptr<SceneRenderer>& Engine::GetEditorSceneRenderer()
	{
		return m_EditorSceneRenderer;
	}

	void Engine::TellToShutdown()
	{
		m_Running = false;
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
		
		if (m_EngineInfo.MaximizeWindow)
		{
			m_Window->MaximizeWindow();
		}

		GameLoop::Instance().Init();
		Shader::SetupShaders();
		RegisterECS();

		m_SceneRenderer = std::make_shared<SceneRenderer>(false);
		m_SceneRenderer->Initialize();
		Renderer::Init();

		if (m_EngineInfo.EnableEditor)
		{
			m_VulkanEditor = std::make_shared<VulkanEditor>(m_Window->GetRenderContext()->GetDeviceInternally());
			m_EditorSceneRenderer = std::make_shared<SceneRenderer>(true);
			m_EditorSceneRenderer->Initialize();
			Engine::GetInstance().GetVulkanImgui()->SetEditorSceneDescriptor(m_EditorSceneRenderer);
		}

		// Init
		Profiler::Instance().StartTimer("InitSystem");
		ECSSystemManager::Instance().InitSystem();
		Profiler::Instance().EndTimer("InitSystem");

		EditorSystemManager::Instance().InitSystem();

		ScriptEngine::Init();
		ScriptEngine::InitScriptingMain();

		if (m_EngineInfo.EnableGame)
		{
			SceneManager::Instance().LoadScene(GETFOLDER(FILESYS_SCENE) + "SplashScreen.json");
			EventHandler::getEventHandlerInstance().Publish(ToggleRunEvent{ true, GameLoop::Instance().GetGameSimulating() });
		}
		else
			SceneManager::Instance().NewScene();
	}

	Engine::~Engine()
	{
		if (!m_Shutdown) Shutdown();
	}

	void Engine::RegisterECS()
	{
		// Load set folders and names
		//FileSystem::Instance().GenerateFolderFileNamesFile("FolderFileNames");

		// Register Components
		ECSManager::Instance().RegisterComponent<Undeployed>("Undeployed", true, false);		// ignore, ignore
		ECSManager::Instance().RegisterComponent<Removal>("Removal", true, false);			// ignore, ignore
		ECSManager::Instance().RegisterComponent<Prefabing>("Prefabing", true, false);		// ignore, ignore
		ECSManager::Instance().RegisterComponent<Properties>("Properties", true, false);		// serialized, reflected
		ECSManager::Instance().RegisterComponent<Parenting>("Parenting", false, false);		// serialized, reflected
		ECSManager::Instance().RegisterComponent<Transform>("Transform", false, false);		// serialized, reflected
		ECSManager::Instance().RegisterComponent<MeshRenderer>("Mesh Renderer");							// 
		ECSManager::Instance().RegisterComponent<Camera>("Camera");											// serialized, reflected
		ECSManager::Instance().RegisterComponent<Rigidbody>("Rigidbody");									// serialized, reflected
		ECSManager::Instance().RegisterComponent<SphereCollider>("SphereCollider");							// serialized, reflected
		ECSManager::Instance().RegisterComponent<BoxCollider>("BoxCollider");								// serialized, reflected
		ECSManager::Instance().RegisterComponent<CapsuleCollider>("CapsuleCollider");						// serialized, reflected
		ECSManager::Instance().RegisterComponent<CylinderCollider>("CylinderCollider");						// serialized, reflected
		ECSManager::Instance().RegisterComponent<Audio>("Audio");											// 
		ECSManager::Instance().RegisterComponent<AudioListener>("AudioListener");							// 
		ECSManager::Instance().RegisterComponent<DirectionalLight>("Directional Light");					// serialized, reflected
		ECSManager::Instance().RegisterComponent<ScriptComponent>("Scripting");								// 
		ECSManager::Instance().RegisterComponent<UIComponent>("UI Component");								// Serialized, reflected
		ECSManager::Instance().RegisterComponent<AnimationComponent>("Animation Component");				// Serialized, reflected
		ECSManager::Instance().RegisterComponent<ParticleComponent>("Particle Component");					// Serialized, reflected
		ECSManager::Instance().RegisterComponent<DirectPathfinding>("Direct Pathfinding");					// Serialized, reflected
		ECSManager::Instance().RegisterComponent<TextComponent>("Text Component");							// Serialized, reflected
		ECSManager::Instance().RegisterComponent<SlideshowComponent>("Slideshow Component");				// Serialized, reflected
		ECSManager::Instance().RegisterComponent<Sprite3DComponent>("Sprite3D Component");					// Serialized, reflected

		// Register Systems
		ECSSystemManager::Instance().RegisterSystem<ScenePostEffectsSystem>();
		ECSSystemManager::Instance().RegisterSystem<PrefabSystem>();
		ECSSystemManager::Instance().RegisterSystem<PhysicsSystem>();
		ECSSystemManager::Instance().RegisterSystem<ParentingSystem>();
		ECSSystemManager::Instance().RegisterSystem<CameraSystem>();
		ECSSystemManager::Instance().RegisterSystem<AudioSystem>();
		ECSSystemManager::Instance().RegisterSystem<MeshRendererSystem>();
		ECSSystemManager::Instance().RegisterSystem<LightSystem>();
		ECSSystemManager::Instance().RegisterSystem<ScriptingSystem>();
		ECSSystemManager::Instance().RegisterSystem<TransformSystem>();
		ECSSystemManager::Instance().RegisterSystem<DirectPathfindingSystem>();
		ECSSystemManager::Instance().RegisterSystem<SlideshowSystem>();
		ECSSystemManager::Instance().RegisterSystem<ParticleSystem>();
		//ECSSystemManager::Instance().RegisterSystem<AnimationSystem>();

		// Allocate Default Size for Memory Manager
		//MemoryManager::Instance().AllocateEntitySize(MemoryManager::Instance().GetConfigSize());
	}

	void Engine::Update()
	{
		while (!m_Window->ShouldWindowClose() && m_Running)
		{
			m_Window->UpdateDeltaTime();

			m_Window->BeginFrame();
			
			Renderer::BeginFrame();

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

				GameLoop::Instance().InstantReset();

				Profiler::Instance().StartTimer("AfterReset");
				ECSSystemManager::Instance().AfterReset();
				Profiler::Instance().EndTimer("AfterReset");

				GameLoop::Instance().SetSceneReset(false);
			}

			Profiler::Instance().StartTimer("OnDestroyEntities");
			ECSSystemManager::Instance().OnDestroyEntities();
			Profiler::Instance().EndTimer("OnDestroyEntities");

			Profiler::Instance().StartTimer("DeleteRemovalEntities");
			ECSManager::Instance().DeleteRemovalEntities();
			Profiler::Instance().EndTimer("DeleteRemovalEntities");

			Renderer::EndFrame();

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

			// This calls the inputHandler to clear the keys
			//InputHandler::ClearKeys();
			
			// THIS IS COMMENTED OUT UNTIL IMGUI IS UP, iteration 1 would be used for displaying until IMGUI can use iteration 2
			Profiler::Instance().PrintTimers();
		}
		
		// If either fails here, shut it down
		Shutdown();
	}

	void Engine::Shutdown()
	{
		m_Running = false;
		ResourceManager::Instance().DestroyResourcesOfType(ResourceType::Texture);
		ResourceManager::Instance().DestroyResourcesOfType(ResourceType::Mesh);
		ResourceManager::Instance().DestroyResourcesOfType(ResourceType::Material);
		ResourceManager::Instance().DestroyResourcesOfType(ResourceType::Shader);
		ResourceManager::Instance().DestroyAllResources();
		EntityCopier::Instance().Shutdown();
		GameLoop::Instance().Shutdown();
		if(m_EngineInfo.EnableEditor)
			EditorCamera::Instance().Shutdown();
		ECSManager::Instance().DestroyAll();
		ECSSystemManager::Instance().ShutdownSystem();
		EditorSystemManager::Instance().ShutdownSystem();
		MemoryManager::Instance().DeleteEntities();
		Renderer::Shutdown();
		m_Shutdown = true;
	}
}