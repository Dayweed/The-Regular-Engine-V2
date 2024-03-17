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
		public bool showGameplayPanel = false;
		public bool showGraphicsPanel = false;
		public bool showAudioPanel = false;
		public bool showControlsPanel = false;

		private bool mIsEditingSettings = false;

		public int mainMenuOption = 0; // for confirmation menu to know which option to execute
		public int currentOption = 0;
		public int menuOption = 1;
		public int menustate = -1; //0 = main menu, 1 = show controls, 2 = confirmation menu
		public int settingsOption = 0;
		public int audioOption = 0;
		public String settings_name = "";

		private ulong sfx;

		// Pause menu pointer
		private Entity pointer;
		private Transform pointerTransform;

		private Entity destructivePointer;
		private Transform destructivePointerTransform;

		private Entity audioPointer;
		private Transform audioPointerTransform;

		// Pause menu options
		private List<Entity> options;
		private List<Entity> DestructiveActionConfirmations;
		private List<Entity> settingsPanel;
		private List<Entity> settingsSelected;
		private List<Entity> audioPanel;

		private Entity pauseMenu;
		private Entity cfmMenu;
		private Entity settingsMenu;

		//Audio panel entities
		private Entity masterVolumeEnt;
		private Entity musicVolumeEnt;
		private Entity sfxVolumeEnt;
		private int masterVolume = 100;
		private int musicVolume = 100;
		private int sfxVolume = 100;

		//Graphics panel entities
		private int mCurrentEditMember = -1;
		private Entity mGammaPanel;
        private Entity mGammaCheckbox;
        private Entity mGammaTick;
        private Entity mGraphicsPointer;
        bool mIsGammaOn = true;
		//private Transform settingsPointerTransform;

		private CameraController mainCamera;

        public void Start()
		{
			options = new List<Entity>();
			options.Add(ECSManager.FindEntityByName("main_continue"));
			options.Add(ECSManager.FindEntityByName("main_controls"));
			options.Add(ECSManager.FindEntityByName("main_quit"));

			DestructiveActionConfirmations = new List<Entity>();
			DestructiveActionConfirmations.Add(ECSManager.FindEntityByName("destructive_yes"));
			DestructiveActionConfirmations.Add(ECSManager.FindEntityByName("destructive_no"));

			//non selected
			settingsPanel = new List<Entity>();
			settingsPanel.Add(ECSManager.FindEntityByName("gameplay_panel"));
			settingsPanel.Add(ECSManager.FindEntityByName("graphics_panel"));
			settingsPanel.Add(ECSManager.FindEntityByName("audio_panel"));
			settingsPanel.Add(ECSManager.FindEntityByName("controls_panel"));

            //selected
            settingsSelected = new List<Entity>();
			settingsSelected.Add(ECSManager.FindEntityByName("gameplay_selected"));
			settingsSelected.Add(ECSManager.FindEntityByName("graphics_selected"));
			settingsSelected.Add(ECSManager.FindEntityByName("audio_selected"));
			settingsSelected.Add(ECSManager.FindEntityByName("controls_selected"));

			sfx = ECSManager.FindIDFromName("PauseMenu");

			//audio panel
            audioPanel = new List<Entity>();
			int total_children = ECSManager.FindEntityByName("audio_selected").parenting.GetTotalChildren();
			for (int i = 0; i < total_children; i++)
			{
				String childName = ECSManager.FindEntityByName("audio_selected").parenting.GetChild(i).name;
				audioPanel.Add(ECSManager.FindEntityByName(childName));
			}
			masterVolumeEnt = ECSManager.FindEntityByName("master_volume");
			musicVolumeEnt = ECSManager.FindEntityByName("music_volume");
			sfxVolumeEnt = ECSManager.FindEntityByName("sfx_volume");
			TextSystem.SetTextMessage(masterVolumeEnt.ID, masterVolume.ToString());
			TextSystem.SetTextMessage(musicVolumeEnt.ID, musicVolume.ToString());
			TextSystem.SetTextMessage(sfxVolumeEnt.ID, sfxVolume.ToString());
			AudioSystem.SetMasterVolume(masterVolume / 100f);

			pointer = ECSManager.FindEntityByName("main_pointer");
			pointerTransform = pointer.GetComponent<Transform>();

			destructivePointer = ECSManager.FindEntityByName("destructive_Pointer");
			destructivePointerTransform = destructivePointer.GetComponent<Transform>();

			audioPointer = ECSManager.FindEntityByName("audio_pointer");
			audioPointerTransform = audioPointer.GetComponent<Transform>();

			pauseMenu = ECSManager.FindEntityByName("PauseMenu");
			cfmMenu = ECSManager.FindEntityByName("pauseMenu_destructive");
			settingsMenu = ECSManager.FindEntityByName("settings_panel");

            mGammaPanel = ECSManager.FindEntityByName("gamma_panel");
			mGammaCheckbox = ECSManager.FindEntityByName("checkbox_gamma");
			mGammaTick = ECSManager.FindEntityByName("tick_gamma");
			mGraphicsPointer = ECSManager.FindEntityByName("graphics_pointer");

			Entity mainCam = ECSManager.FindEntityByName("Main Camera");
			if(mainCam != null)
			{
				mainCamera = mainCam.GetComponent<CameraController>();
			}
			else
			{
				mainCamera = new CameraController();
				mainCamera.freeCamera = true;
			}
		}

		public void Update()
		{
			if ((InputSystem.GetKeyTriggered(InputKeys.Escape) && mainCamera.freeCamera)
				|| InputSystem.GetControllerButtonTriggered(0, InputSystem.Button.Start) 
				|| InputSystem.GetControllerButtonTriggered(1, InputSystem.Button.Start))
			{
				if (menustate == -1)
				{
                    isChangeMenu = true;
                    isPaused = !isPaused;
                    menustate = 0;
                    currentOption = 0;
                    Debug.Log("Trigger: Go into pause state");
                }
				else if (mIsEditingSettings && menustate == 1)
				{
                    isChangeMenu = true;
                    menustate = 1;
					mIsEditingSettings = false;
					ResetPointers();
                    Debug.Log("Trigger: Get out of editing state");
                }
                else if (!mIsEditingSettings && menustate == 1)
                {
                    isChangeMenu = true;
                    menustate = 0;
					BacktoMainPausePage();
                    Debug.Log("Trigger: Get out of controls panel");
                }
                else if (!mIsEditingSettings && menustate == 0)
				{
					isChangeMenu = true;
                    isPaused = !isPaused;
                    menustate = 0;
                    currentOption = 0;
                    Debug.Log("Trigger: Unpause the game");
                }

				if (ECSManager.IsValidEntity(sfx))
				{
					AudioSystem.Play(sfx);
				}

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

						if (ECSManager.IsValidEntity(sfx))
						{
							AudioSystem.Play(sfx);
						}
					}

					if (InputSystem.GetKeyTriggered(InputKeys.S) || ControllerInput(MenuNavigation.DOWN))
					{
						if (currentOption == 2)
							currentOption = 0;
						else
							currentOption += 1;

						if (ECSManager.IsValidEntity(sfx))
						{
							AudioSystem.Play(sfx);
						}
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
							isChangeMenu = true;
						}
						else if (currentOption == 2) // Quit Game
						{
							isConfirming = true;
							mainMenuOption = 2;
							menustate = 2;
							currentOption = 1;
							isChangeMenu = true;
						}

						if (ECSManager.IsValidEntity(sfx))
						{
							AudioSystem.Play(sfx);
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
                //Not editing settings + show panels individually
                else if (menustate == 1 && !mIsEditingSettings) // control menu logic
                {
					//settings pop up will appear
					//user can press A or D to move left or right for "Gameplay", "Graphics", "Audio", "Controls"
					if (InputSystem.GetKeyTriggered(InputKeys.A))
					{
						Debug.Log("press left");
                        Debug.Log("mCurrentEditMember: " + mCurrentEditMember);
                        if (settingsOption < 0)
							settingsOption = 0;
						else if (settingsOption > 3)
							settingsOption = 3;
						else
							--settingsOption;

						if (ECSManager.IsValidEntity(sfx))
						{
							AudioSystem.Play(sfx);
						}
					}

					if (InputSystem.GetKeyTriggered(InputKeys.D))
					{
						Debug.Log("press right");
                        Debug.Log("mCurrentEditMember: " + mCurrentEditMember);
                        if (settingsOption < 0)
							settingsOption = 0;
						else if (settingsOption > 3)
							settingsOption = 3;
						else
							++settingsOption;

						if (ECSManager.IsValidEntity(sfx))
						{
							AudioSystem.Play(sfx);
						}
					}

					switch (settingsOption)
					{
						case 0:
							settings_name = "Gameplay";
							showGameplayPanel = true;
							showGraphicsPanel = false;
							showAudioPanel = false;
							showControlsPanel = false;
                            break;
						case 1:
							settings_name = "Graphics";
							showGameplayPanel = false;
							showGraphicsPanel = true;
							showAudioPanel = false;
							showControlsPanel = false;
                            break;
						case 2:
							settings_name = "Audio";
							showGameplayPanel = false;
							showGraphicsPanel = false;
							showAudioPanel = true;
							showControlsPanel = false;
                            break;
						case 3:
							settings_name = "Controls";
							showGameplayPanel = false;
							showGraphicsPanel = false;
							showAudioPanel = false;
							showControlsPanel = true;
                            break;
						default:
							showGameplayPanel = false;
							showGraphicsPanel = false;
							showAudioPanel = false;
							showControlsPanel = false;
							break;
					}

					//show which panel is selected
					if (settingsOption >= 0 && settingsOption <= 3)
					{
						for (int i = 0; i < settingsSelected.Count; ++i)
						{
							//other 3 selected == false
							if (i != settingsOption)
							{
								UISystem.SetVisible(settingsSelected[i].ID, false);
							}
							else
							{
								UISystem.SetVisible(settingsSelected[settingsOption].ID, true);
							}
						}

						for (int i = 0; i < settingsPanel.Count; ++i)
						{
							//other 3 panel == true
							if (i != settingsOption)
							{
								UISystem.SetVisible(settingsPanel[i].ID, true);
							}
							else
							{
								UISystem.SetVisible(settingsPanel[settingsOption].ID, false);
							}
						}
					}

					//show audio panel
					for (int i = 0; i < audioPanel.Count; ++i)
					{
						//show text
						if (audioPanel[i].HasComponent<Text>())
						{
							TextSystem.SetVisible(audioPanel[i].ID, showAudioPanel);
						}
						//show UI
						else
						{
                            UISystem.SetVisible(audioPanel[i].ID, showAudioPanel);
						}
					}
					audioPointer.GetComponent<SpriteRenderer>().isVisible = false;

					//Not editing settings -> editing settings
					//lock this action for gameplay and controls (havent implemented)
					if (!mIsEditingSettings && InputSystem.GetKeyTriggered(InputKeys.S) && (showGraphicsPanel || showAudioPanel))
					{
						mIsEditingSettings = true;
						mCurrentEditMember = 0; //Set it to be 0th member always at the start

                        Debug.Log("1st trigger mCurrentEditMember: " + mCurrentEditMember);
                    }

					//Show graphics panel
					mGammaPanel.GetComponent<SpriteRenderer>().isVisible = showGraphicsPanel;
					mGammaCheckbox.GetComponent<SpriteRenderer>().isVisible = showGraphicsPanel;
					if (mIsGammaOn)
					{
						mGammaTick.GetComponent<SpriteRenderer>().isVisible = showGraphicsPanel;
					}
				}
				//edit settings here
				else if (menustate == 1 && mIsEditingSettings)
				{
					if (showAudioPanel)
					{
                        audioPointer.GetComponent<SpriteRenderer>().isVisible = true;
                        if (InputSystem.GetKeyTriggered(InputKeys.W))
                        {
                            Debug.Log("press up");
                            if (mCurrentEditMember <= 0)
							{
								mCurrentEditMember = -1;
								mIsEditingSettings = false;
							}
							else if (mCurrentEditMember > 2)
								mCurrentEditMember = 2;
							else
								--mCurrentEditMember;

                            Debug.Log("mCurrentEditMember: " + mCurrentEditMember);
                        }

                        if (InputSystem.GetKeyTriggered(InputKeys.S))
                        {
                            Debug.Log("press down");
                            if (mCurrentEditMember < 0)
								mIsEditingSettings = false;
							else if (mCurrentEditMember >= 2)
								mCurrentEditMember = 2;
							else
								++mCurrentEditMember;

                            Debug.Log("mCurrentEditMember: " + mCurrentEditMember);
                        }

						//adjust each setting
						switch (mCurrentEditMember)
						{
							case 0:
								if (InputSystem.GetKeyPress(InputKeys.A) || InputSystem.GetKeyPress(InputKeys.Left))
								{
									masterVolume -= 10;

									if (masterVolume < 0)
										masterVolume = 0;

								}
								else if (InputSystem.GetKeyPress(InputKeys.D) || InputSystem.GetKeyPress(InputKeys.Right))
								{
									masterVolume += 10;

									if (masterVolume > 100)
										masterVolume = 100;
								}
								AudioSystem.SetMasterVolume(masterVolume / 100f);
								TextSystem.SetTextMessage(masterVolumeEnt.ID, masterVolume.ToString());
								break;
							case 1:
								if (InputSystem.GetKeyPress(InputKeys.A) || InputSystem.GetKeyPress(InputKeys.Left))
								{
									musicVolume -= 10;

									if (musicVolume < 0)
										musicVolume = 0;

								}
								else if (InputSystem.GetKeyPress(InputKeys.D) || InputSystem.GetKeyPress(InputKeys.Right))
								{
									musicVolume += 10;

									if (musicVolume > 100)
										musicVolume = 100;
								}
								AudioSystem.SetBGMVolume(musicVolume / 100f);
								TextSystem.SetTextMessage(musicVolumeEnt.ID, musicVolume.ToString());
								break;
							case 2:
								if (InputSystem.GetKeyPress(InputKeys.A) || InputSystem.GetKeyPress(InputKeys.Left))
								{
									sfxVolume -= 10;

									if (sfxVolume < 0)
										sfxVolume = 0;

								}
								else if (InputSystem.GetKeyPress(InputKeys.D) || InputSystem.GetKeyPress(InputKeys.Right))
								{
									sfxVolume += 10;

									if (sfxVolume > 100)
										sfxVolume = 100;
								}
								AudioSystem.SetSFXVolume(sfxVolume / 100f);
								TextSystem.SetTextMessage(sfxVolumeEnt.ID, sfxVolume.ToString());
								break;
						}

						//move pointer
						switch (mCurrentEditMember)
                        {
                            case 0:
                                audioPointerTransform.Position = audioPanel[0].GetComponent<Transform>().Position;
                                break;
                            case 1:
                                audioPointerTransform.Position = audioPanel[1].GetComponent<Transform>().Position;
                                break;
                            case 2:
                                audioPointerTransform.Position = audioPanel[2].GetComponent<Transform>().Position;
                                break;
                        }
                    }
					else if (showGraphicsPanel)
					{
						mGraphicsPointer.GetComponent<SpriteRenderer>().isVisible = true;
						if (mCurrentEditMember == 0)
						{
                            mGraphicsPointer.GetComponent<Transform>().Position = mGammaPanel.GetComponent<Transform>().Position;

                            if (InputSystem.GetKeyTriggered(InputKeys.Enter))
							{
								mIsGammaOn = !mIsGammaOn;
								if (mIsGammaOn)
								{
                                    Game.TurnGammaOn();
                                    mGammaTick.GetComponent<SpriteRenderer>().isVisible = true;
                                }
								else
								{
									Game.TurnGammaOff();
                                    mGammaTick.GetComponent<SpriteRenderer>().isVisible = false;
                                }
							}
						}
						else if (mCurrentEditMember == 1) //Can be smth else in future
						{

						}
					}
					else if (showControlsPanel)
					{

					}
					else if (showGameplayPanel)
					{

					}

                    if (InputSystem.GetKeyTriggered(InputKeys.W) || InputSystem.GetControllerButtonTriggered(0, InputSystem.Button.Start) || InputSystem.GetControllerButtonTriggered(1, InputSystem.Button.Start))
                    {
                        Debug.Log("Triggered ESC to not editing settings");
                        mIsEditingSettings = false; //Set to not editing any option
                        menustate = 1;

                        //Set every pointer back to false
                        mGraphicsPointer.GetComponent<SpriteRenderer>().isVisible = false;
						audioPointer.GetComponent<SpriteRenderer>().isVisible = false;

						if (ECSManager.IsValidEntity(sfx))
						{
							AudioSystem.Play(sfx);
						}
					}
                }
                else // confirmation menu logic which is menustate == 2
                {
					if (InputSystem.GetKeyTriggered(InputKeys.A) || ControllerInput(MenuNavigation.LEFT))
					{
						if (menuOption == 0)
							menuOption = 1;
						else if (menuOption == 1)
							menuOption = 0;

						if (ECSManager.IsValidEntity(sfx))
						{
							AudioSystem.Play(sfx);
						}
					}

					if (InputSystem.GetKeyTriggered(InputKeys.D) || ControllerInput(MenuNavigation.RIGHT))
					{
						if (menuOption == 0)
							menuOption = 1;
						else if (menuOption == 1)
							menuOption = 0;

						if (ECSManager.IsValidEntity(sfx))
						{
							AudioSystem.Play(sfx);
						}
					}

					if (InputSystem.GetKeyTriggered(InputKeys.Enter) || ControllerInput(MenuNavigation.CONFIRM))
					{
						if (menuOption == 0) // yes
						{
							// Quit game
							//Game.CloseGame();

							// go back to the main menu
							Scene.TransitionScene("MainMenu", 4.5f);
						}
						else if (menuOption == 1) // no
						{
							isConfirming = false;
							menustate = 0;
							isChangeMenu = true;
							menuOption = 1;
						}
						if (ECSManager.IsValidEntity(sfx))
						{
							AudioSystem.Play(sfx);
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
							//show settings menu
							UISystem.SetVisible(settingsMenu.ID, true);

							//                     //set confirmation stuff as invisible
							//                     UISystem.SetVisible(cfmMenu.ID, false);
							//for (int i = 0; i < DestructiveActionConfirmations.Count; i++)
							//{
							//	UISystem.SetVisible(DestructiveActionConfirmations[i].ID, false);
							//}
							//UISystem.SetVisible(destructivePointer.ID, false);
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
			else if (isChangeMenu && !isPaused) //This is when you are getting out of the pause menu
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

				UISystem.SetVisible(settingsMenu.ID, false);
				for (int i = 0; i < settingsPanel.Count; i++)
				{
					UISystem.SetVisible(settingsPanel[i].ID, false);
					UISystem.SetVisible(settingsSelected[i].ID, false);
				}

                for (int i = 0; i < audioPanel.Count; ++i)
                {
                    //show text
                    if (audioPanel[i].HasComponent<Text>())
                    {
                        TextSystem.SetVisible(audioPanel[i].ID, false);
                    }

                    //show UI
                    else
                    {
                        UISystem.SetVisible(audioPanel[i].ID, false);
                    }
                }

				UISystem.SetVisible(mGammaTick.ID, false);
                UISystem.SetVisible(mGammaPanel.ID, false);
                UISystem.SetVisible(mGammaCheckbox.ID, false);

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

		private void BacktoMainPausePage()
		{
			for (int x = 0; x < settingsSelected.Count; x++)
			{
				UISystem.SetVisible(settingsSelected[x].ID, false);
			}

            for (int x = 0; x < settingsPanel.Count; x++)
            {
                UISystem.SetVisible(settingsPanel[x].ID, false);
            }

            UISystem.SetVisible(settingsMenu.ID, false);

			if (showGraphicsPanel)
			{
                UISystem.SetVisible(mGammaPanel.ID, false);
                UISystem.SetVisible(mGammaCheckbox.ID, false);
                UISystem.SetVisible(mGammaTick.ID, false);
                UISystem.SetVisible(mGraphicsPointer.ID, false);
				showGraphicsPanel = false;
            }

			if (showAudioPanel)
			{
				for (int x = 0; x < audioPanel.Count; x++)
				{
                    if (audioPanel[x].HasComponent<Text>())
                    {
                        TextSystem.SetVisible(audioPanel[x].ID, false);
                    }
                    UISystem.SetVisible(audioPanel[x].ID, false);
				}

                showAudioPanel = false;
			}

            if (showControlsPanel)
            {

                showControlsPanel = false;
            }

            if (showGameplayPanel)
            {

                showGameplayPanel = false;
            }
        }

		private void ResetPointers()
		{
            if (showGraphicsPanel)
            {
                UISystem.SetVisible(mGraphicsPointer.ID, false);
            }

            if (showAudioPanel)
            {
                UISystem.SetVisible(audioPointer.ID, false);
            }

            if (showControlsPanel)
            {
				//Change below to the correct pointer and delete this comment
                //UISystem.SetVisible(audioPointer.ID, false);
            }

            if (showGameplayPanel)
            {
                //Change below to the correct pointer and delete this comment
                //UISystem.SetVisible(audioPointer.ID, false);
            }
        }
	}
}
