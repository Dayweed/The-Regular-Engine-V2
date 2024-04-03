using System.Runtime.Serialization.Formatters;

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
		public bool ControllerAPressed;
		public bool ControllerBPressed;

		public bool ControllerConnected;
		private Entity StartText;
		private bool changeUI = false;

		public void Start()
		{
			nextSceneName = "MainMenu";
			//ControlsDisplaySceneName = "ControlsDisplay";
			spacePressed = false;
			enterPressed = false;
			ControllerAPressed = false;
			ControllerBPressed = false;

			StartText = ECSManager.FindEntityByName("Start_Text");

			if (IS.GetControllerConnected(0) || IS.GetControllerConnected(1))
			{
				ControllerConnected = true;
				// change the texture for the ui display
				StartText.GetComponent<SpriteRenderer>().Texture = "ui-start-controller.png";

			}
			else
			{
				ControllerConnected = false;
			}
		}

		public void Update()
		{

			// check for controller hotplug
			if ((IS.GetControllerConnected(0) || IS.GetControllerConnected(1)) && changeUI == false)
			{
				ControllerConnected = true;
				changeUI = true;
			}
			else if ((!IS.GetControllerConnected(0) || !IS.GetControllerConnected(1)) && changeUI == false)
			{
				ControllerConnected = false;
				changeUI = true;
			}

			if (changeUI)
			{
				if (ControllerConnected)
				{
					StartText.GetComponent<SpriteRenderer>().Texture = "ui-start-controller.png";
				}
				else
				{
					StartText.GetComponent<SpriteRenderer>().Texture = "ui-start.png";
				}
				changeUI = false;
			}


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

			if (IS.GetControllerButtonTriggered(0, IS.Button.A) || IS.GetControllerButtonTriggered(1, IS.Button.A))
			{
				ControllerAPressed = true;
				if (ECSManager.IsValidEntity(13376208322872696703))
					AS.Play(13376208322872696703);
			}
			if (IS.GetControllerButtonTriggered(0, IS.Button.B) || IS.GetControllerButtonTriggered(1, IS.Button.B))
			{
				ControllerBPressed = true;
				if (ECSManager.IsValidEntity(13376208322872696703))
					AS.Play(13376208322872696703);
			}

			if ((spacePressed && enterPressed) || (ControllerAPressed && ControllerBPressed))
			{
				if (ECSManager.IsValidEntity(6154957411926925810))
					AS.Play(6154957411926925810);

				Scene.TransitionScene(nextSceneName, 5.0f);

				spacePressed = false;
				enterPressed = false;
				ControllerAPressed = false;
				ControllerBPressed = false;
			}
		}
	}
}
