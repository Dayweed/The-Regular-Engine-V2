using System;
using System.Collections.Generic;

namespace TRE
{
	public enum MenuNavigation
	{
		UP,
		DOWN,
		LEFT,
		RIGHT,
		CONFIRM,
		BACK
	}

	public class PauseMenu : Entity
	{
		public bool isPaused = false;
		public bool isConfirming = false;
		public bool isChangeMenu = false;

		public int mainMenuOption = 0; // for confirmation menu to know which option to execute
		public int currentOption = 0;
		public int menuOption = 1;
		public int menustate = 0; //0 = main menu, 1 = show controls,2 = confirmation menu
								  //public int settingsOption = 0;
								  //public String settings_name = "";

		////Settings -> Gameplay
		//public int gameplayOption = 0;

		////Settings -> Graphics
		//public int graphicsOption = 0;
		//public int displayOption = 0;
		//public int framerateOption = 0;
		//public int vsyncOption = 0;
		//public int resolutionOption = 0;

		////Settings -> Audio
		//public int audioOption = 0;
		//      public int masterVol = 0;
		//      public int bgmVol = 0;
		//      public int sfxVol = 0;

		//      //Settings -> Controls
		//      public int controlsOption = 0;


		// Pause menu pointer
		private Entity pointer;
		private Transform pointerTransform;

		private Entity destructivePointer;
		private Transform destructivePointerTransform;

		// Pause menu options
		private List<Entity> options;
		private List<Entity> DestructiveActionConfirmations;
		//private List<Entity> settings;

		private Entity pauseMenu;
		private Entity cfmMenu;
		//private Entity settingsMenu;

		//private Entity settingsPointer;
		//private Transform settingsPointerTransform;

		public void Start()
		{
			options = new List<Entity>();
			DestructiveActionConfirmations = new List<Entity>();
			//settings = new List<Entity>();

			pointer = ECSManager.FindEntityByName("main_pointer");
			pointerTransform = pointer.GetComponent<Transform>();

			options.Add(ECSManager.FindEntityByName("main_continue"));
			options.Add(ECSManager.FindEntityByName("main_controls"));
			options.Add(ECSManager.FindEntityByName("main_quit"));

			destructivePointer = ECSManager.FindEntityByName("destructive_Pointer");
			destructivePointerTransform = destructivePointer.GetComponent<Transform>();

			DestructiveActionConfirmations.Add(ECSManager.FindEntityByName("destructive_yes"));
			DestructiveActionConfirmations.Add(ECSManager.FindEntityByName("destructive_no"));

			pauseMenu = ECSManager.FindEntityByName("PauseMenu");
			cfmMenu = ECSManager.FindEntityByName("pauseMenu_destructive");

			//settings.Add(ECSManager.FindEntityByName("settings_gameplay"));
			//settings.Add(ECSManager.FindEntityByName("settings_graphics"));
			//settings.Add(ECSManager.FindEntityByName("settings_audio"));
			//settings.Add(ECSManager.FindEntityByName("settings_controls"));

			//put buttons + pointer inside
			//settingsMenu = ECSManager.FindEntityByName("pauseMenu_controls");

			//assumes that the size of the pointer are all the same
			//settingsPointer = ECSManager.FindEntityByName("settingsPointer");
			//settingsPointerTransform = settingsPointer.GetComponent<Transform>();
		}

		public void OnCreate()
		{

		}

		public void Update()
		{
			if (InputSystem.GetKeyTriggered(InputKeys.Escape) || InputSystem.GetControllerButtonTriggered(0, InputSystem.Button.Start) || InputSystem.GetControllerButtonTriggered(1, InputSystem.Button.Start))
			{
				isPaused = !isPaused;
				isChangeMenu = true;
				menustate = 0;
				currentOption = 0;
			}

			//HARD CODE FOR NOW
			String levelName = Scene.GetSceneName();
			if (levelName == "MainMenu" || levelName == "ResultScreen")
			{
				SilhouetteEffect.Engine_SetSilhouetteActive(false);
			}
			else if (levelName == "Tutorial" || levelName == "Level1" || levelName == "Level2")
			{
				SilhouetteEffect.Engine_SetSilhouetteActive(!isPaused);
			}

			if (isPaused)
			{
				// Pause menu logic
				if (menustate == 0)
				{
					//we are assuming that pause menu entering is handled by game logic
					if (InputSystem.GetKeyTriggered(InputKeys.W) || ControllerInput(MenuNavigation.UP))
					{
						if (currentOption == 0)
							currentOption = 2;
						else
							currentOption -= 1;
					}

					if (InputSystem.GetKeyTriggered(InputKeys.S) || ControllerInput(MenuNavigation.DOWN))
					{
						if (currentOption == 2)
							currentOption = 0;
						else
							currentOption += 1;
					}

					if (InputSystem.GetKeyTriggered(InputKeys.Enter) || ControllerInput(MenuNavigation.CONFIRM))
					{
						if (currentOption == 0) // resume game
						{
							isPaused = false;
							isChangeMenu = true;
							menustate = 0;
							currentOption = 0;
						}
						else if (currentOption == 1) // Controls
						{
							menustate = 1;
						}
						else if (currentOption == 2) // Quit Game
						{
							isConfirming = true;
							mainMenuOption = 2;
							menustate = 2;
							currentOption = 1;
							isChangeMenu = true;
						}
					}

					switch (currentOption)
					{
						case 0: // resume
							pointerTransform.Position = options[0].GetComponent<Transform>().Position;
							break;
						case 1: // 
							pointerTransform.Position = options[1].GetComponent<Transform>().Position;
							break;
						case 2:
							pointerTransform.Position = options[2].GetComponent<Transform>().Position;
							break;
					}
				}

				// control menu logic
				else if (menustate == 1)
				{
					//               //settings pop up will appear
					//               //user can press A or D to move left or right for "Gameplay", "Graphics", "Audio", "Controls"
					//               if (InputSystem.GetKeyTriggered(InputKeys.A))
					//               {
					//                   if (settingsOption == 0)
					//                       settingsOption = 3;
					//                   else
					//		--settingsOption;
					//               }

					//               if (InputSystem.GetKeyTriggered(InputKeys.D))
					//               {
					//                   if (settingsOption == 3)
					//                       settingsOption = 0;
					//                   else
					//		++settingsOption;
					//               }

					//               //set settings_name accordingly
					//               switch (settingsOption)
					//               {
					//                   case 0: //Gameplay
					//                       settings_name = "Gameplay";
					//                       break;
					//                   case 1: //Graphics
					//                       settings_name = "Graphics";
					//                       break;
					//                   case 2: //Audio
					//                       settings_name = "Audio";
					//                       break;
					//                   case 3: //Controls
					//                       settings_name = "Controls";
					//                       break;
					//               }

					//               //user can press W or S to move up or down in its own settings
					//               if (InputSystem.GetKeyTriggered(InputKeys.W))
					//               {
					//                   switch (settings_name)
					//                   {
					//                       //? types of settings
					//                       case "Gameplay":
					//                           //if (gameplayOption == 0)
					//                           //    gameplayOption = 3;
					//                           //else
					//                           //    --gameplayOption;
					//                           break;

					//		//4 types of settings
					//                       case "Graphics":
					//                           if (graphicsOption == 0)
					//                               graphicsOption = 3;
					//                           else
					//                               --graphicsOption;
					//                           break;

					//                       //3 types of settings
					//                       case "Audio":
					//                           if (audioOption == 0)
					//                               audioOption = 2;
					//                           else
					//                               --audioOption;
					//                           break;

					//                       //? types of settings
					//                       case "Controls":
					//                           //if (controlsOption == 0)
					//                           //    controlsOption = 3;
					//                           //else
					//                           //    --controlsOption;
					//                           break;
					//                   }
					//               }

					//               if (InputSystem.GetKeyTriggered(InputKeys.S))
					//               {
					//                   switch (settings_name)
					//                   {
					//                       //? types of settings
					//                       case "Gameplay":
					//                           //if (gameplayOption == 3)
					//                           //    gameplayOption = 0;
					//                           //else
					//                           //    ++gameplayOption;
					//                           break;

					//		//4 types of settings
					//                       case "Graphics":
					//                           if (graphicsOption == 3)
					//                               graphicsOption = 0;
					//                           else
					//                               ++graphicsOption;
					//                           break;

					//		//3 types of settings
					//                       case "Audio":
					//                           if (audioOption == 2)
					//                               audioOption = 0;
					//                           else
					//                               ++audioOption;
					//                           break;

					//                       //? types of settings
					//                       case "Controls":
					//                           //if (controlsOption == 3)
					//                           //    controlsOption = 0;
					//                           //else
					//                           //	++controlsOption;
					//                           break;
					//                   }
					//               }

					//               if (settings_name == "Gameplay")
					//               {
					//                   switch (gameplayOption)
					//                   {
					//                       case 0: //?
					//                           break;
					//                       case 1: //?
					//                           break;
					//                       case 2: //?
					//                           break;
					//                       case 3: //?
					//                           break;
					//                   }
					//               }

					//               else if (settings_name == "Graphics")
					//{
					//	switch (graphicsOption)
					//	{
					//		case 0: //Display Mode
					//			break;
					//		case 1: //Frame Rate
					//			break; 
					//		case 2: //VSync
					//			break;
					//		case 3: //Resolution
					//			break;
					//	}
					//}

					//               else if (settings_name == "Audio")
					//               {
					//                   switch (audioOption)
					//                   {
					//                       case 0: //Master Volume
					//                           break;
					//                       case 1: //Music Volume
					//                           break;
					//                       case 2: //SFX Volume
					//                           break;
					//                   }
					//               }

					//               else if (settings_name == "Controls")
					//               {
					//                   switch (controlsOption)
					//                   {
					//                       case 0: //?
					//                           break;
					//                       case 1: //?
					//                           break;
					//                       case 2: //?
					//                           break;
					//                       case 3: //?
					//                           break;
					//                   }
					//               }
				}

				// confirmation menu logic
				else
				{
					if (InputSystem.GetKeyTriggered(InputKeys.A) || ControllerInput(MenuNavigation.LEFT))
					{
						if (menuOption == 0)
							menuOption = 1;
						else if (menuOption == 1)
							menuOption = 0;
					}

					if (InputSystem.GetKeyTriggered(InputKeys.D) || ControllerInput(MenuNavigation.RIGHT))
					{
						if (menuOption == 0)
							menuOption = 1;
						else if (menuOption == 1)
							menuOption = 0;
					}

					if (InputSystem.GetKeyTriggered(InputKeys.Enter) || ControllerInput(MenuNavigation.CONFIRM))
					{
						if (menuOption == 0) // yes
						{
							// Quit game
							Game.CloseGame();
						}
						else if (menuOption == 1) // no
						{
							isConfirming = false;
							menustate = 0;
							isChangeMenu = true;
							menuOption = 1;
						}
					}

					switch (menuOption)
					{
						case 0: // yes
							destructivePointerTransform.Position = DestructiveActionConfirmations[0].GetComponent<Transform>().Position;
							break;
						case 1: // no
							destructivePointerTransform.Position = DestructiveActionConfirmations[1].GetComponent<Transform>().Position;
							break;
					}

				}


				if (isChangeMenu && isPaused)
				{
					// change menu visibilities
					switch (menustate)
					{
						case 0:
							// Show main pause menu
							UISystem.SetVisible(pauseMenu.ID, true);
							UISystem.SetVisible(cfmMenu.ID, false);
							for (int i = 0; i < options.Count; i++)
							{
								UISystem.SetVisible(options[i].ID, true);
							}
							for (int i = 0; i < DestructiveActionConfirmations.Count; i++)
							{
								UISystem.SetVisible(DestructiveActionConfirmations[i].ID, false);
							}
							// show the pointer
							UISystem.SetVisible(pointer.ID, true);
							UISystem.SetVisible(destructivePointer.ID, false);
							break;
						case 1:
							//set settings popup as visible


							//set confirmation stuff as invisible
							UISystem.SetVisible(cfmMenu.ID, false);
							for (int i = 0; i < DestructiveActionConfirmations.Count; i++)
							{
								UISystem.SetVisible(DestructiveActionConfirmations[i].ID, false);
							}
							UISystem.SetVisible(destructivePointer.ID, false);
							break;
						case 2:
							// only show the confirmation menu since destructive action UI is transparent
							UISystem.SetVisible(cfmMenu.ID, true);
							for (int i = 0; i < DestructiveActionConfirmations.Count; i++)
							{
								UISystem.SetVisible(DestructiveActionConfirmations[i].ID, true);
							}
							// show the pointer
							UISystem.SetVisible(destructivePointer.ID, true);
							break;
					}
					isChangeMenu = false;
				}
			}

			else if (isChangeMenu && !isPaused)
			{
				// hide the whole pause menu
				UISystem.SetVisible(pauseMenu.ID, false);
				UISystem.SetVisible(cfmMenu.ID, false);

				// hide the pointer
				UISystem.SetVisible(pointer.ID, false);
				UISystem.SetVisible(destructivePointer.ID, false);

				for (int i = 0; i < options.Count; i++)
				{
					UISystem.SetVisible(options[i].ID, false);
				}

				for (int i = 0; i < DestructiveActionConfirmations.Count; i++)
				{
					UISystem.SetVisible(DestructiveActionConfirmations[i].ID, false);
				}

				isChangeMenu = false;

			}
		}

		private bool ControllerInput(MenuNavigation button)
		{

			float leftStickY_0 = InputSystem.GetControllerStickY(0, false);
			float leftStickY_1 = InputSystem.GetControllerStickY(1, false);

			// Check if the controller input is valid
			switch (button)
			{
				case MenuNavigation.UP:
					if (InputSystem.GetControllerButtonTriggered(0, InputSystem.Button.DPadUp) || InputSystem.GetControllerButtonTriggered(1, InputSystem.Button.DPadUp))
						return true;
					// handle analog stick
					if (leftStickY_0 > 0.5f || leftStickY_1 > 0.5f)
						return true;
					break;
				case MenuNavigation.DOWN:
					if (InputSystem.GetControllerButtonTriggered(0, InputSystem.Button.DPadDown) || InputSystem.GetControllerButtonTriggered(1, InputSystem.Button.DPadDown))
						return true;
					// handle analog stick
					if (leftStickY_0 < -0.5f || leftStickY_1 < -0.5f)
						return true;
					break;
				case MenuNavigation.LEFT:
					if (InputSystem.GetControllerButtonTriggered(0, InputSystem.Button.DPadLeft) || InputSystem.GetControllerButtonTriggered(1, InputSystem.Button.DPadLeft))
						return true;
					// handle analog stick
					if (InputSystem.GetControllerStickX(0, false) < -0.5f || InputSystem.GetControllerStickX(1, false) < -0.5f)
						return true;
					break;
				case MenuNavigation.RIGHT:
					if (InputSystem.GetControllerButtonTriggered(0, InputSystem.Button.DPadRight) || InputSystem.GetControllerButtonTriggered(1, InputSystem.Button.DPadRight))
						return true;
					// handle analog stick
					if (InputSystem.GetControllerStickX(0, false) > 0.5f || InputSystem.GetControllerStickX(1, false) > 0.5f)
						return true;
					break;
				case MenuNavigation.CONFIRM:
					if (InputSystem.GetControllerButtonTriggered(0, InputSystem.Button.A) || InputSystem.GetControllerButtonTriggered(1, InputSystem.Button.A))
						return true;
					break;
				case MenuNavigation.BACK:
					if (InputSystem.GetControllerButtonTriggered(0, InputSystem.Button.B) || InputSystem.GetControllerButtonTriggered(1, InputSystem.Button.B))
						return true;
					break;
			}
			return false;
		}

		public bool IsPaused()
		{
			return isPaused;
		}
	}
}
