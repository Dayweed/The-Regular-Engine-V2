namespace TRE
{
	using AS = AudioSystem;
	using IS = InputSystem;

	public class MainMenuSceneTransition : Entity
	{
		private string nextSceneName;
		//private string ControlsDisplaySceneName;
		public bool spacePressed;
		public bool enterPressed;
		public bool ContollerAPressed;

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
			if (IS.GetKeyHold(InputKeys.Escape))
			{
				Game.CloseGame();
			}

			if (IS.GetKeyPress(InputKeys.Space))
			{
				spacePressed = true;
				if (ECSManager.IsValidEntity(13376208322872696703))
					AS.Play(13376208322872696703);
			}

			if (IS.GetKeyPress(InputKeys.Enter))
			{
				enterPressed = true;
				if (ECSManager.IsValidEntity(13376208322872696703))
					AS.Play(13376208322872696703);
			}

			if (IS.GetControllerButtonPress(0, IS.Button.A))
			{
				ContollerAPressed = true;
			}

			if ((spacePressed && enterPressed) || ContollerAPressed)
			{
				if (ECSManager.IsValidEntity(6154957411926925810))
					AS.Play(6154957411926925810);

				Scene.TransitionScene(nextSceneName, 5.0f);

				spacePressed = false;
				enterPressed = false;
			}
		}
	}
}
