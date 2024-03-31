namespace TRE
{
	public class EffectsManager : Entity
	{
		PauseMenu pauseMenu = null;
		CameraController cameraController = null;
		string levelName;
		float depthBlurThreshold = 0.9997f; //Default value - Smaller the value the nearer the blur effect is to the camera -- try small increments

		public void Start()
		{
			levelName = Scene.GetSceneName();
			// Debug.Log("Level Name is " + levelName);

			Entity tempPause = ECSManager.FindEntityByName("PauseMenu");
			if (tempPause != null)
			{
				pauseMenu = tempPause.GetComponent<PauseMenu>();
			}

			Entity tempTransitions = ECSManager.FindEntityByName("Main Camera");
			if (tempTransitions != null)
			{
				if (levelName == "MainMenu" || levelName == "ResultScreen")
				{
					cameraController = null;
				}
				else if (levelName == "Tutorial" || levelName == "Level_1" || levelName == "Level_2")
				{
					cameraController = tempTransitions.GetComponent<CameraController>();
				}
				else
				{
					cameraController = null;
					Debug.Log("Transitions is null");
				}
			}

			//Depth Blur
			if (levelName == "MainMenu")
			{
				//depthBlurThreshold = 0.0f;
			}
			else if (levelName == "ResultScreen")
			{
				//depthBlurThreshold = 0.0f;
			}
			else if (levelName == "Tutorial")
			{
				//depthBlurThreshold = 0.0f;
			}
			else if (levelName == "Level_1")
			{
				//depthBlurThreshold = 0.39997f;
			}
			else if (levelName == "Level_2")
			{
				//depthBlurThreshold = 0.0f;
			}

			DepthBlurEffect.Engine_SetDepthBlurThreshold(depthBlurThreshold);
		}

		public void Update()
		{
			if (levelName == "MainMenu" || levelName == "ResultScreen")
			{
				SilhouetteEffect.Engine_SetSilhouetteActive(false);
				DepthBlurEffect.Engine_SetDepthBlurActive(false);
			}
			else if (levelName == "Tutorial" || levelName == "Level_1" || levelName == "Level_2")
			{
				if (cameraController != null)
				{
					if (cameraController.freeCamera == false)
					{
						SilhouetteEffect.Engine_SetSilhouetteActive(false);
					}
					else
					{
						if (pauseMenu != null)
							SilhouetteEffect.Engine_SetSilhouetteActive(!pauseMenu.isPaused);
					}
				}
				else
				{
					if (pauseMenu != null)
						SilhouetteEffect.Engine_SetSilhouetteActive(!pauseMenu.isPaused);
				}

				//If camera following player, enable depth blur
				if(cameraController.freeCamera)
					DepthBlurEffect.Engine_SetDepthBlurActive(!pauseMenu.isPaused);
				else
					DepthBlurEffect.Engine_SetDepthBlurActive(false);
			}
		}
	}
}
