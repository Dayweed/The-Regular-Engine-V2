using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;

namespace TRE
{
	public class MainMenuSceneTransition : Entity
	{
		private string TutorialSceneName;
		//private string ControlsDisplaySceneName;
		public bool spacePressed;
		public bool enterPressed;

		public void Start()
		{
			TutorialSceneName = "Tutorial";
			//ControlsDisplaySceneName = "ControlsDisplay";
			spacePressed = false;
			enterPressed = false;
		}

		public void Update()
		{
			// Close Game
			if (InputSystem.GetKeyDown(InputKeys.Escape))
			{
				Game.CloseGame();
			}

			if (InputSystem.GetKeyTrigger(InputKeys.Space))
			{
				spacePressed = true;
				if (ECSManager.IsValidEntity(13376208322872696703))
				{
					AudioSystem.Play(13376208322872696703);
				}
			}

			if (InputSystem.GetKeyTrigger(InputKeys.Enter))
			{
				enterPressed = true;
				if (ECSManager.IsValidEntity(13376208322872696703))
				{
					AudioSystem.Play(13376208322872696703);
				}
			}

			if (spacePressed && enterPressed)
			{
				if (ECSManager.IsValidEntity(6154957411926925810))
				{
					AudioSystem.Play(6154957411926925810);
				}

				Scene.TransitionScene(TutorialSceneName, 4.0f);

				spacePressed = false;
				enterPressed = false;
			}
		}
	}
}
