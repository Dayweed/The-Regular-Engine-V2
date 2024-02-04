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
		private string nextSceneName;
		//private string ControlsDisplaySceneName;
		public bool spacePressed;
		public bool enterPressed;

		public void Start()
		{
			nextSceneName = "MainMenu";
			//ControlsDisplaySceneName = "ControlsDisplay";
			spacePressed = false;
			enterPressed = false;
		}

		public void Update()
		{
			// Close Game
			if (InputSystem.GetKeyHold(InputKeys.Escape))
			{
				Game.CloseGame();
			}

			if (InputSystem.GetKeyPress(InputKeys.Space))
			{
				spacePressed = true;
				if (ECSManager.IsValidEntity(13376208322872696703))
				{
					AudioSystem.Play(13376208322872696703);
				}
			}

			if (InputSystem.GetKeyPress(InputKeys.Enter))
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

				Scene.TransitionScene(nextSceneName, 5.0f);

				spacePressed = false;
				enterPressed = false;
			}
		}
	}
}
