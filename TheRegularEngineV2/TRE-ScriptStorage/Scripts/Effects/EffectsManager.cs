namespace TRE
{
	public class EffectsManager : Entity
	{
		PauseMenu pauseMenu = null;
		CameraController cameraController = null;
		string levelName;

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
				DepthBlurEffect.Engine_SetDepthBlurActive(!pauseMenu.isPaused);
			}
		}
	}
}
