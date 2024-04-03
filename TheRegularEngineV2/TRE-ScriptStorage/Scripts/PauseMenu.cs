using System.Collections.Generic;

namespace TRE
{
	using AS = AudioSystem;
	using IS = InputSystem;
	using TS = TextSystem;
	using UIS = UISystem;
	using PRS = PersistentSystem;

	public enum MenuNavigation
	{
		UP,
		DOWN,
		LEFT,
		RIGHT,
		CONFIRM,
		BACK,
		START,
		SELECT
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
		public bool playUISound = false;

		private bool mIsEditingSettings = false;

		public int mainMenuOption = 0; // for confirmation menu to know which option to execute
		public int currentOption = 0;
		public int menuOption = 1;
		public int menustate = -1; //0 = main menu, 1 = show controls, 2 = confirmation menu
		public int settingsOption = 0;
		public int audioOption = 0;
		public string settings_name = "";

		private ulong sfx;

		// Pause menu pointer
		private Entity pointer;
		private Transform pointerTransform;

		private Entity destructivePointer;
		private Transform destructivePointerTransform;

		private Entity settingsPointer;
		private Transform settingsPointerTransform;

		private Entity controlsPointer;
		private Transform controlsPointerTransform;

		// Pause menu options
		private List<Entity> options;
		private List<Entity> DestructiveActionConfirmations;
		private List<Entity> settingsPanel;
		private List<Entity> settingsSelected;
		private List<Entity> audioPanel;

		private Entity pauseMenu;
		private Entity cfmMenu;
		private Entity settingsMenu;
		private Entity settingsClose;

		//Gameplay panel entities
		private Entity power_ups_panel;
		private Entity on_button_pwrUp;
		private Entity on_button_backing_pwrUp;
		private Entity off_button_pwrUp;
		private Entity off_button_backing_pwrUp;
		private Entity invulnerability_panel;
		private Entity on_button_inv;
		private Entity on_button_backing_inv;
		private Entity off_button_inv;
		private Entity off_button_backing_inv;
		// Set In persistent system
		bool IsPowerUpOn = false;
		bool IsInvulnerabilityOn = false;

		//Audio panel entities
		private Entity masterVolumeEnt;
		private Entity musicVolumeEnt;
		private Entity sfxVolumeEnt;
		// Need to store these values into the persistent system
		public int masterVolume = 100;
        public int musicVolume = 100;
        public int sfxVolume = 100;

		//Graphics panel entities
		private int mCurrentEditMember = -1;
		private Entity mGammaPanel;
		private Entity mGammaValue;
		// store to persistentsystem
        public float mTempGammaValue = 2.2f; //  Start at 2.2

		//Controls panel entities
		private Entity P1_panel;
		private Entity P1_keyboard_panel;
		private Entity P1_keyboard_preset1;
		private Entity P1_keyboard_preset2;
		private Entity P1_keyboard_preset3;
		private Entity P1_controller_panel;
		private Entity P1_controller_preset1;
		private Entity P1_controller_preset2;
		private Entity P2_panel;
		private Entity P2_keyboard_panel;
		private Entity P2_keyboard_preset1;
		private Entity P2_keyboard_preset2;
		private Entity P2_keyboard_preset3;
		private Entity P2_controller_panel;
		private Entity P2_controller_preset1;
		private Entity P2_controller_preset2;
		private bool P1_IsKeyboard = true;
		private bool P2_IsKeyboard = true;
		private int P1_KeyboardPreset = 0;
		private int P2_KeyboardPreset = 1;
		private int P1_ControllerPreset = 0;
		private int P2_ControllerPreset = 0;

		private CameraController mainCamera;
		private string CurrentScene;

		private bool controllerConnected = false;
		private bool controllerConnectedLast = false;
		private bool changeUI = false;

		private Entity moley;
		private Entity holey;

		private Entity scenelogic;

		private bool externalPauseCommand = false;

		public void Start()
		{
			moley = ECSManager.FindEntityByName("Moley");
			holey = ECSManager.FindEntityByName("Holey");
			scenelogic = ECSManager.FindEntityByName("Scene Transition Logic");

			options = new List<Entity>
			{
				ECSManager.FindEntityByName("main_continue"),
				ECSManager.FindEntityByName("main_controls"),
				ECSManager.FindEntityByName("main_quit")
			};

			DestructiveActionConfirmations = new List<Entity>
			{
				ECSManager.FindEntityByName("destructive_yes"),
				ECSManager.FindEntityByName("destructive_no")
			};

			//non selected
			settingsPanel = new List<Entity>
			{
				ECSManager.FindEntityByName("gameplay_panel"),
				ECSManager.FindEntityByName("graphics_panel"),
				ECSManager.FindEntityByName("audio_panel"),
				ECSManager.FindEntityByName("controls_panel")
			};

			//selected
			settingsSelected = new List<Entity>
			{
				ECSManager.FindEntityByName("gameplay_selected"),
				ECSManager.FindEntityByName("graphics_selected"),
				ECSManager.FindEntityByName("audio_selected"),
				ECSManager.FindEntityByName("controls_selected")
			};

			sfx = ECSManager.FindIDFromName("PauseMenu");

			//audio panel
			{
				audioPanel = new List<Entity>();
				int total_children = ECSManager.FindEntityByName("audio_selected").parenting.GetTotalChildren();
				for (int i = 0; i < total_children; i++)
				{
					string childName = ECSManager.FindEntityByName("audio_selected").parenting.GetChild(i).name;
					audioPanel.Add(ECSManager.FindEntityByName(childName));
				}
				masterVolumeEnt = ECSManager.FindEntityByName("master_volume");
				musicVolumeEnt = ECSManager.FindEntityByName("music_volume");
				sfxVolumeEnt = ECSManager.FindEntityByName("sfx_volume");
				TS.SetTextMessage(masterVolumeEnt.ID, masterVolume.ToString());
				TS.SetTextMessage(musicVolumeEnt.ID, musicVolume.ToString());
				TS.SetTextMessage(sfxVolumeEnt.ID, sfxVolume.ToString());
				AS.SetMasterVolume(masterVolume / 100f);
			}

			//gameplay panel
			{
				power_ups_panel = ECSManager.FindEntityByName("power_ups_panel");
				on_button_pwrUp = ECSManager.FindEntityByName("on_button_pwrUp");
				on_button_backing_pwrUp = ECSManager.FindEntityByName("on_button_backing_pwrUp");
				off_button_pwrUp = ECSManager.FindEntityByName("off_button_pwrUp");
				off_button_backing_pwrUp = ECSManager.FindEntityByName("off_button_backing_pwrUp");
				invulnerability_panel = ECSManager.FindEntityByName("invulnerability_panel");
				on_button_inv = ECSManager.FindEntityByName("on_button_inv");
				on_button_backing_inv = ECSManager.FindEntityByName("on_button_backing_inv");
				off_button_inv = ECSManager.FindEntityByName("off_button_inv");
				off_button_backing_inv = ECSManager.FindEntityByName("off_button_backing_inv");
				settingsClose = ECSManager.FindEntityByName("close_settings");
			}

			//graphics panel
			{
				mGammaPanel = ECSManager.FindEntityByName("gamma_panel");
				mGammaValue = ECSManager.FindEntityByName("gamma_value");
			}

			//controls panel
			{
				P1_panel = ECSManager.FindEntityByName("P1_panel");
				P2_panel = ECSManager.FindEntityByName("P2_panel");

				P1_keyboard_panel = ECSManager.FindEntityByName("P1_keyboard_panel");
				P1_keyboard_preset1 = ECSManager.FindEntityByName("P1_keyboard_preset1");
				P1_keyboard_preset2 = ECSManager.FindEntityByName("P1_keyboard_preset2");
				P1_keyboard_preset3 = ECSManager.FindEntityByName("P1_keyboard_preset3");

				P1_controller_panel = ECSManager.FindEntityByName("P1_controller_panel");
				P1_controller_preset1 = ECSManager.FindEntityByName("P1_controller_preset1");
				P1_controller_preset2 = ECSManager.FindEntityByName("P1_controller_preset2");

				P2_keyboard_panel = ECSManager.FindEntityByName("P2_keyboard_panel");
				P2_keyboard_preset1 = ECSManager.FindEntityByName("P2_keyboard_preset1");
				P2_keyboard_preset2 = ECSManager.FindEntityByName("P2_keyboard_preset2");
				P2_keyboard_preset3 = ECSManager.FindEntityByName("P2_keyboard_preset3");

				P2_controller_panel = ECSManager.FindEntityByName("P2_controller_panel");
				P2_controller_preset1 = ECSManager.FindEntityByName("P2_controller_preset1");
				P2_controller_preset2 = ECSManager.FindEntityByName("P2_controller_preset2");
			}
			pointer = ECSManager.FindEntityByName("main_pointer");
			pointerTransform = pointer.GetComponent<Transform>();

			destructivePointer = ECSManager.FindEntityByName("destructive_Pointer");
			destructivePointerTransform = destructivePointer.GetComponent<Transform>();

			settingsPointer = ECSManager.FindEntityByName("settings_pointer");
			settingsPointerTransform = settingsPointer.GetComponent<Transform>();

			controlsPointer = ECSManager.FindEntityByName("ControlsPointer");
			controlsPointerTransform = controlsPointer.GetComponent<Transform>();

			pauseMenu = ECSManager.FindEntityByName("PauseMenu");
			cfmMenu = ECSManager.FindEntityByName("pauseMenu_destructive");
			settingsMenu = ECSManager.FindEntityByName("settings_panel");

			// Get the current scene 
			CurrentScene = Scene.GetSceneName();
			if (CurrentScene == "MainMenu")
			{
				// Hide the quit button
				options[2].GetComponent<SpriteRenderer>().Texture = "ui-button-quit.png";
			}


			Entity mainCam = ECSManager.FindEntityByName("Main Camera");
			if (mainCam != null)
			{
				mainCamera = mainCam.GetComponent<CameraController>();
			}
			else
			{
				mainCamera = new CameraController { freeCamera = true };
			}

			#region Load Settings
			// Get the current settings
			// Master Volume
			if (int.TryParse(PRS.GetValue("masterVolume"), out int result))
			{
				masterVolume = result;
				AS.SetMasterVolume(masterVolume / 100f);
				TS.SetTextMessage(masterVolumeEnt.ID, masterVolume.ToString());
			}
			else
			{
				masterVolume = 100;
				string masterVolumeString = masterVolume.ToString();
				PRS.SetValue("masterVolume", masterVolumeString);
			}

			// Music Volume
			if (int.TryParse(PRS.GetValue("musicVolume"), out result))
			{
				musicVolume = result;
				AS.SetBGMVolume(musicVolume / 100f);
				TS.SetTextMessage(musicVolumeEnt.ID, musicVolume.ToString());
			}
			else
			{
				musicVolume = 100;
				string musicVolumeString = musicVolume.ToString();
				PRS.SetValue("musicVolume", musicVolumeString);
			}

			// SFX Volume
			if (int.TryParse(PRS.GetValue("sfxVolume"), out result))
			{
				sfxVolume = result;
				AS.SetSFXVolume(sfxVolume / 100f);
				TS.SetTextMessage(sfxVolumeEnt.ID, sfxVolume.ToString());
			}
			else
			{
				sfxVolume = 100;
				string sfxVolumeString = sfxVolume.ToString();
				PRS.SetValue("sfxVolume", sfxVolumeString);
			}

			// Gamma Value
			if (float.TryParse(PRS.GetValue("gammaValue"), out float gammaResult))
			{
				mTempGammaValue = gammaResult;
				Game.SetGammaValue(mTempGammaValue);
				TS.SetTextMessage(mGammaValue.ID, mTempGammaValue.ToString());
			}
			else
			{
				mTempGammaValue = 2.2f;
				string gammaValueString = mTempGammaValue.ToString();
				PRS.SetValue("gammaValue", gammaValueString);
			}

			// Cheats
			// Power Ups
			if (bool.TryParse(PRS.GetValue("powerUps"), out bool powerUpsResult))
			{
				IsPowerUpOn = powerUpsResult;
			}
			else
			{
				IsPowerUpOn = false;
				string powerUpsString = IsPowerUpOn.ToString();
				PRS.SetValue("powerUps", powerUpsString);
			}

			// Invulnerability
			if (bool.TryParse(PRS.GetValue("invulnerability"), out bool invulnerabilityResult))
			{
				IsInvulnerabilityOn = invulnerabilityResult;
			}
			else
			{
				IsInvulnerabilityOn = false;
				string invulnerabilityString = IsInvulnerabilityOn.ToString();
				PRS.SetValue("invulnerability", invulnerabilityString);
			}

			#endregion
		}

		public void Update()
		{
			bool playerPressedPause = IS.GetKeyPress(InputKeys.Escape) ||
				IS.GetControllerButtonTriggered(0, IS.Button.Start) ||
				IS.GetControllerButtonTriggered(1, IS.Button.Start);

			// Is controller connected
			controllerConnected = IS.GetControllerConnected(0) || IS.GetControllerConnected(1);

			P1_IsKeyboard = !IS.GetControllerConnected(0);
			P2_IsKeyboard = !IS.GetControllerConnected(1);

			if (controllerConnected != controllerConnectedLast)
			{
				changeUI = true;
				controllerConnectedLast = controllerConnected;
			}

			if (controllerConnected && changeUI)
			{
				// change the texture for the settings_close button
				//settingsClose.GetComponent<SpriteRenderer>().Texture = "ui-button-back-controller.png";
				changeUI = false;
			}
			else if (!controllerConnected && changeUI)
			{
				// change the texture for the settings_close button
				//settingsClose.GetComponent<SpriteRenderer>().Texture = "ui-button-back.png";
				changeUI = false;
			}


			// prevent pause menu from showing up in the level camera pans
			// but allow it to show up it the MainMenu scene even if its camera is not free
			bool canShowPauseMenu = CurrentScene == "MainMenu" || mainCamera.freeCamera;

			if ((playerPressedPause || ReadExternalPauseCommand()) && canShowPauseMenu)
			{
				if (menustate == -1)
				{
					isChangeMenu = true;
					isPaused = !isPaused;
					menustate = 0;
					currentOption = 0;
					//Debug.Log("Trigger: Go into pause state");
				}
				else if (mIsEditingSettings && menustate == 1)
				{
					isChangeMenu = true;
					menustate = 1;
					mIsEditingSettings = false;
					UIS.SetVisible(settingsPointer.ID, false);
					UIS.SetVisible(controlsPointer.ID, false);
					//Debug.Log("Trigger: Get out of editing state");
				}
				else if (!mIsEditingSettings && menustate == 1)
				{
					isChangeMenu = true;
					menustate = 0;
					BacktoMainPausePage();
					//Debug.Log("Trigger: Get out of controls panel");
				}
				else if (!mIsEditingSettings && menustate == 0)
				{
					isChangeMenu = true;
					isPaused = !isPaused;
					menustate = 0;
					currentOption = 0;
					//Debug.Log("Trigger: Unpause the game");
				}

				if (ECSManager.IsValidEntity(sfx))
					AS.Play(sfx);
			}

			if (isPaused)
			{
				//if (IS.GetKeyPress(InputKeys.Escape) || IS.GetKeyPress(InputKeys.Tab) || IS.GetKeyPress(InputKeys.Space) || IS.GetKeyPress(InputKeys.W) || IS.GetKeyPress(InputKeys.A) || IS.GetKeyPress(InputKeys.S) || IS.GetKeyPress(InputKeys.D)
				//	|| IS.GetKeyPress(InputKeys.I) || IS.GetKeyPress(InputKeys.J) || IS.GetKeyPress(InputKeys.K) || IS.GetKeyPress(InputKeys.L) || IS.GetKeyPress(InputKeys.Enter)
				//	|| ControllerInput(MenuNavigation.UP) || ControllerInput(MenuNavigation.DOWN) || ControllerInput(MenuNavigation.LEFT) || ControllerInput(MenuNavigation.RIGHT) || ControllerInput(MenuNavigation.CONFIRM) || ControllerInput(MenuNavigation.SELECT)
				//	|| IS.GetControllerButtonTriggered(0, IS.Button.Start) || IS.GetControllerButtonTriggered(1, IS.Button.Start))
				//{
				//	if (ECSManager.IsValidEntity(sfx))
				//		AS.Play(sfx);
				//}
				// Pause menu logic
				if (menustate == 0)
				{
					//we are assuming that pause menu entering is handled by game logic
					if (IS.GetKeyPress(InputKeys.W) || ControllerInput(MenuNavigation.UP))
					{
						if (currentOption == 0)
							currentOption = 2;
						else
							currentOption -= 1;

                        playUISound = true;
                    }

					if (IS.GetKeyPress(InputKeys.S) || ControllerInput(MenuNavigation.DOWN))
					{
						if (currentOption == 2)
							currentOption = 0;
						else
							currentOption += 1;

                        playUISound = true;
					}

					if (IS.GetKeyPress(InputKeys.Enter) || ControllerInput(MenuNavigation.CONFIRM))
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

                        playUISound = true;
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
					if (IS.GetKeyPress(InputKeys.A) || ControllerInput(MenuNavigation.LEFT))
					{
						//Debug.Log("press left");
						//Debug.Log("mCurrentEditMember: " + mCurrentEditMember);
						if (settingsOption <= 0)
							settingsOption = 0;
						else if (settingsOption > 3)
							settingsOption = 3;
						else
							--settingsOption;

                        playUISound = true;
					}

					if (IS.GetKeyPress(InputKeys.D) || ControllerInput(MenuNavigation.RIGHT))
					{
						//Debug.Log("press right");
						//Debug.Log("mCurrentEditMember: " + mCurrentEditMember);
						if (settingsOption < 0)
							settingsOption = 0;
						else if (settingsOption >= 3)
							settingsOption = 3;
						else
							++settingsOption;

                        playUISound = true;
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
								UIS.SetVisible(settingsSelected[i].ID, false);
							else
								UIS.SetVisible(settingsSelected[settingsOption].ID, true);
						}

						for (int i = 0; i < settingsPanel.Count; ++i)
						{
							//other 3 panel == true
							if (i != settingsOption)
								UIS.SetVisible(settingsPanel[i].ID, true);
							else
								UIS.SetVisible(settingsPanel[settingsOption].ID, false);
						}
					}

					//don't show pointer when switching panels
					settingsPointer.GetComponent<SpriteRenderer>().isVisible = false;
					controlsPointer.GetComponent<SpriteRenderer>().isVisible = false;

					//show gameplay panel
					power_ups_panel.GetComponent<SpriteRenderer>().isVisible = showGameplayPanel;
					invulnerability_panel.GetComponent<SpriteRenderer>().isVisible = showGameplayPanel;
					if (IsPowerUpOn)
					{
						on_button_pwrUp.GetComponent<SpriteRenderer>().isVisible = showGameplayPanel;
						on_button_backing_pwrUp.GetComponent<SpriteRenderer>().isVisible = showGameplayPanel;
					}
					else
					{
						off_button_pwrUp.GetComponent<SpriteRenderer>().isVisible = showGameplayPanel;
						off_button_backing_pwrUp.GetComponent<SpriteRenderer>().isVisible = showGameplayPanel;
					}
					if (IsInvulnerabilityOn)
					{
						on_button_inv.GetComponent<SpriteRenderer>().isVisible = showGameplayPanel;
						on_button_backing_inv.GetComponent<SpriteRenderer>().isVisible = showGameplayPanel;
					}
					else
					{
						off_button_inv.GetComponent<SpriteRenderer>().isVisible = showGameplayPanel;
						off_button_backing_inv.GetComponent<SpriteRenderer>().isVisible = showGameplayPanel;
					}

					//show audio panel
					for (int i = 0; i < audioPanel.Count; ++i)
					{
						//show text
						if (audioPanel[i].HasComponent<Text>())
							TS.SetVisible(audioPanel[i].ID, showAudioPanel);
						//show UI
						else
							UIS.SetVisible(audioPanel[i].ID, showAudioPanel);
					}

					//Show graphics panel
					mGammaPanel.GetComponent<SpriteRenderer>().isVisible = showGraphicsPanel;
					mGammaValue.GetComponent<Text>().IsVisible = showGraphicsPanel;

					//show controls panel
					P1_panel.GetComponent<SpriteRenderer>().isVisible = showControlsPanel;
					P2_panel.GetComponent<SpriteRenderer>().isVisible = showControlsPanel;
					showControlPanel();

					//Not editing settings -> editing settings
					if (!mIsEditingSettings && (IS.GetKeyPress(InputKeys.S) || ControllerInput(MenuNavigation.DOWN)))
					{
						mIsEditingSettings = true;
						mCurrentEditMember = 0; //Set it to be 0th member always at the start
                        playUISound = true;
						//Debug.Log("1st trigger mCurrentEditMember: " + mCurrentEditMember);
					}
				}
				//edit settings here
				else if (menustate == 1 && mIsEditingSettings)
				{
					if (showAudioPanel)
					{
						settingsPointer.GetComponent<SpriteRenderer>().isVisible = true;
						if (IS.GetKeyPress(InputKeys.W) || ControllerInput(MenuNavigation.UP))
						{
							//Debug.Log("press up");
							if (mCurrentEditMember <= 0)
							{
								mCurrentEditMember = -1;
								mIsEditingSettings = false;
							}
							else if (mCurrentEditMember > 2)
								mCurrentEditMember = 2;
							else
								--mCurrentEditMember;
                            
                            playUISound = true;
							//Debug.Log("mCurrentEditMember: " + mCurrentEditMember);
						}

						if (IS.GetKeyPress(InputKeys.S) || ControllerInput(MenuNavigation.DOWN))
						{
							//Debug.Log("press down");
							if (mCurrentEditMember < 0)
								mIsEditingSettings = false;
							else if (mCurrentEditMember >= 2)
								mCurrentEditMember = 2;
							else
								++mCurrentEditMember;
                            playUISound = true;

							//Debug.Log("mCurrentEditMember: " + mCurrentEditMember);
						}

						//adjust each setting
						switch (mCurrentEditMember)
						{
							case 0:
								if (IS.GetKeyPress(InputKeys.A) || IS.GetKeyPress(InputKeys.Left) || ControllerInput(MenuNavigation.LEFT))
								{
									masterVolume -= 10;

									if (masterVolume < 0)
										masterVolume = 0;

									// Set persistent system
									string masterVolumeString = masterVolume.ToString();
									PRS.SetValue("masterVolume", masterVolumeString);
                                    playUISound = true;
								}
								else if (IS.GetKeyPress(InputKeys.D) || IS.GetKeyPress(InputKeys.Right) || ControllerInput(MenuNavigation.RIGHT))
								{
									masterVolume += 10;

									if (masterVolume > 100)
										masterVolume = 100;

									// Set persistent system
									string masterVolumeString = masterVolume.ToString();
									PRS.SetValue("masterVolume", masterVolumeString);
                                    playUISound = true;
								}
								AS.SetMasterVolume(masterVolume / 100f);
								TS.SetTextMessage(masterVolumeEnt.ID, masterVolume.ToString());
								break;
							case 1:
								if (IS.GetKeyPress(InputKeys.A) || IS.GetKeyPress(InputKeys.Left) || ControllerInput(MenuNavigation.LEFT))
								{
									musicVolume -= 10;

									if (musicVolume < 0)
										musicVolume = 0;

									// Set persistent system
									string musicVolumeString = musicVolume.ToString();
									PRS.SetValue("musicVolume", musicVolumeString);
                                    playUISound = true;

								}
								else if (IS.GetKeyPress(InputKeys.D) || IS.GetKeyPress(InputKeys.Right) || ControllerInput(MenuNavigation.RIGHT))
								{
									musicVolume += 10;

									if (musicVolume > 100)
										musicVolume = 100;

									// Set persistent system
									string musicVolumeString = musicVolume.ToString();
									PRS.SetValue("musicVolume", musicVolumeString);
                                    playUISound = true;
								}
								AS.SetBGMVolume(musicVolume / 100f);
								TS.SetTextMessage(musicVolumeEnt.ID, musicVolume.ToString());
								break;
							case 2:
								if (IS.GetKeyPress(InputKeys.A) || IS.GetKeyPress(InputKeys.Left) || ControllerInput(MenuNavigation.LEFT))
								{
									sfxVolume -= 10;

									if (sfxVolume < 0)
										sfxVolume = 0;

									// Set persistent system
									string sfxVolumeString = sfxVolume.ToString();
									PRS.SetValue("sfxVolume", sfxVolumeString);

                                    playUISound = true;
								}
								else if (IS.GetKeyPress(InputKeys.D) || IS.GetKeyPress(InputKeys.Right) || ControllerInput(MenuNavigation.RIGHT))
								{
									sfxVolume += 10;

									if (sfxVolume > 100)
										sfxVolume = 100;

									//
									string sfxVolumeString = sfxVolume.ToString();
									PRS.SetValue("sfxVolume", sfxVolumeString);
                                    playUISound = true;
								}

								AS.SetSFXVolume(sfxVolume / 100f);
								TS.SetTextMessage(sfxVolumeEnt.ID, sfxVolume.ToString());
								break;
						}

						//move pointer
						switch (mCurrentEditMember)
						{
							case 0:
								settingsPointerTransform.Position = audioPanel[0].GetComponent<Transform>().Position;
								break;
							case 1:
								settingsPointerTransform.Position = audioPanel[1].GetComponent<Transform>().Position;
								break;
							case 2:
								settingsPointerTransform.Position = audioPanel[2].GetComponent<Transform>().Position;
								break;
						}
					}
					else if (showGraphicsPanel)
					{
						settingsPointer.GetComponent<SpriteRenderer>().isVisible = true;
						if (IS.GetKeyPress(InputKeys.W) || ControllerInput(MenuNavigation.UP))
						{
							//Debug.Log("press up");
							if (mCurrentEditMember <= 0)
							{
								mCurrentEditMember = -1;
								mIsEditingSettings = false;
							}
							else
								--mCurrentEditMember;

                            playUISound = true;
							//Debug.Log("mCurrentEditMember: " + mCurrentEditMember);
						}

						if (mCurrentEditMember == 0)
						{
							settingsPointerTransform.Position = mGammaPanel.GetComponent<Transform>().Position;
							if (IS.GetKeyPress(InputKeys.A) || IS.GetKeyPress(InputKeys.Left))
							{
								mTempGammaValue -= 0.1f;
								if (mTempGammaValue < 1f)
									mTempGammaValue = 1f;
								Game.SetGammaValue(mTempGammaValue);
								TS.SetTextMessage(mGammaValue.ID, mTempGammaValue.ToString("F1"));

								// Set persistent system
								string gammaValueString = mTempGammaValue.ToString("F1");
								PRS.SetValue("gammaValue", gammaValueString);
                                playUISound = true;

							}
							else if (IS.GetKeyPress(InputKeys.D) || IS.GetKeyPress(InputKeys.Right))
							{
								mTempGammaValue += 0.1f;
								Game.SetGammaValue(mTempGammaValue);
								TS.SetTextMessage(mGammaValue.ID, mTempGammaValue.ToString("F1"));

								// Set persistent system
								string gammaValueString = mTempGammaValue.ToString("F1");
								PRS.SetValue("gammaValue", gammaValueString);
                                playUISound = true;
							}

						}
						else if (mCurrentEditMember == 1) //Can be smth else in future
						{

						}
					}
					else if (showControlsPanel)
					{
						//show thick boy (pointer)
						controlsPointer.GetComponent<SpriteRenderer>().isVisible = true;

						if (IS.GetKeyPress(InputKeys.W) || ControllerInput(MenuNavigation.UP))
						{
							//Debug.Log("press up");
							mCurrentEditMember = -1;
							mIsEditingSettings = false;
                            playUISound = true;
							//Debug.Log("mCurrentEditMember: " + mCurrentEditMember);
						}

						if (IS.GetKeyPress(InputKeys.Tab) || ControllerInput(MenuNavigation.SELECT))
						{
							if (mCurrentEditMember == 0)
								mCurrentEditMember = 1;
							else if (mCurrentEditMember == 1)
								mCurrentEditMember = 0;
                            playUISound = true;
							//Debug.Log("mCurrentEditMember: " + mCurrentEditMember);
						}

						switch (mCurrentEditMember)
						{
							case 0: //player 1
								if (P1_IsKeyboard)
								{
									if (IS.GetKeyPress(InputKeys.A) || IS.GetKeyPress(InputKeys.D) || ControllerInput(MenuNavigation.LEFT) || ControllerInput(MenuNavigation.RIGHT))
									{
										//Debug.Log("in controls panel");
										if (P2_KeyboardPreset == 0)
										{
											if (P1_KeyboardPreset == 1)
												P1_KeyboardPreset = 2;
											else
												P1_KeyboardPreset = 1;
										}

										if (P2_KeyboardPreset == 1)
										{
											if (P1_KeyboardPreset == 2)
												P1_KeyboardPreset = 0;
											else
												P1_KeyboardPreset = 2;
										}

										if (P2_KeyboardPreset == 2)
										{
											if (P1_KeyboardPreset == 1)
												P1_KeyboardPreset = 0;
											else
												P1_KeyboardPreset = 1;
										}

										// Set the persistent system
										string P1_KeyboardPresetString = P1_KeyboardPreset.ToString();
										PRS.SetValue("MoleyKB", P1_KeyboardPresetString);
										moley.GetComponent<MoleyController>().KeyboardPreset = P1_KeyboardPreset;
										moley.GetComponent<MoleyController>().SetKeyboardPreset(P1_KeyboardPreset);
                                        playUISound = true;
									}
								}
								else
								{
									if (ControllerInput(MenuNavigation.LEFT) || ControllerInput(MenuNavigation.RIGHT) ||
										IS.GetKeyPress(InputKeys.A) || IS.GetKeyPress(InputKeys.D))
									{
										if (P1_ControllerPreset == 0)
										{
											P1_ControllerPreset = 1;
										}
										else if (P1_ControllerPreset == 1)
										{
											P1_ControllerPreset = 0;
										}

										// Set the persistent system

										string P1_ControllerPresetString = P1_ControllerPreset.ToString();
										PRS.SetValue("MoleyController", P1_ControllerPresetString);
										moley.GetComponent<MoleyController>().ControllerPreset = P1_ControllerPreset;
										moley.GetComponent<MoleyController>().SetControllerPreset(P1_ControllerPreset);
                                        playUISound = true;

									}
								}
								break;
							case 1: //player 2
								if (P2_IsKeyboard)
								{
									if (IS.GetKeyPress(InputKeys.A) || IS.GetKeyPress(InputKeys.D) || ControllerInput(MenuNavigation.LEFT) || ControllerInput(MenuNavigation.RIGHT))
									{
										//Debug.Log("in controls panel");
										if (P1_KeyboardPreset == 0)
										{
											if (P2_KeyboardPreset == 1)
												P2_KeyboardPreset = 2;
											else
												P2_KeyboardPreset = 1;
										}

										if (P1_KeyboardPreset == 1)
										{
											if (P2_KeyboardPreset == 2)
												P2_KeyboardPreset = 0;
											else
												P2_KeyboardPreset = 2;
										}

										if (P1_KeyboardPreset == 2)
										{
											if (P2_KeyboardPreset == 1)
												P2_KeyboardPreset = 0;
											else
												P2_KeyboardPreset = 1;
										}

										// Set the persistent system
										string P2_KeyboardPresetString = P2_KeyboardPreset.ToString();
										PRS.SetValue("HoleyKB", P2_KeyboardPresetString);
										holey.GetComponent<HoleyController>().KeyboardPreset = P2_KeyboardPreset;
										holey.GetComponent<HoleyController>().SetKeyboardPreset(P2_KeyboardPreset);
                                        playUISound = true;

										//Debug.Log("P2_KeyboardPreset: " + P2_KeyboardPreset);
									}
								}
								else
								{
									if (ControllerInput(MenuNavigation.LEFT) || ControllerInput(MenuNavigation.RIGHT) ||
										IS.GetKeyPress(InputKeys.A) || IS.GetKeyPress(InputKeys.D))
									{
										if (P2_ControllerPreset == 0)
										{
											P2_ControllerPreset = 1;
										}
										else if (P2_ControllerPreset == 1)
										{
											P2_ControllerPreset = 0;
										}

										// Set the persistent system
										string P2_ControllerPresetString = P2_ControllerPreset.ToString();
										PRS.SetValue("HoleyController", P2_ControllerPresetString);
										holey.GetComponent<HoleyController>().ControllerPreset = P2_ControllerPreset;
										holey.GetComponent<HoleyController>().SetControllerPreset(P2_ControllerPreset);
                                        playUISound = true;
									}
								}
								break;
						}

						showControlPanel();

						//move pointer
						switch (mCurrentEditMember)
						{
							case 0:
								controlsPointerTransform.Position = P1_keyboard_panel.GetComponent<Transform>().Position;
								break;
							case 1:
								controlsPointerTransform.Position = P2_keyboard_panel.GetComponent<Transform>().Position;
								break;
						}
					}
					else if (showGameplayPanel)
					{
						settingsPointer.GetComponent<SpriteRenderer>().isVisible = true;
						if (IS.GetKeyPress(InputKeys.W) || ControllerInput(MenuNavigation.UP))
						{
							//Debug.Log("press up");
							if (mCurrentEditMember <= 0)
							{
								mCurrentEditMember = -1;
								mIsEditingSettings = false;
							}
							else if (mCurrentEditMember > 1)
								mCurrentEditMember = 1;
							else
								--mCurrentEditMember;

                            playUISound = true;
							//Debug.Log("mCurrentEditMember: " + mCurrentEditMember);
						}

						if (IS.GetKeyPress(InputKeys.S) || ControllerInput(MenuNavigation.DOWN))
						{
							//Debug.Log("press down");
							if (mCurrentEditMember < 0)
								mIsEditingSettings = false;
							else if (mCurrentEditMember >= 1)
								mCurrentEditMember = 1;
							else
								++mCurrentEditMember;

                            playUISound = true;
							//Debug.Log("mCurrentEditMember: " + mCurrentEditMember);
						}

						//adjust each setting
						switch (mCurrentEditMember)
						{
							case 0:
								if (IS.GetKeyPress(InputKeys.A) || IS.GetKeyPress(InputKeys.Left) || ControllerInput(MenuNavigation.LEFT))
								{
									//on
									IsPowerUpOn = true;
									on_button_pwrUp.GetComponent<SpriteRenderer>().isVisible = IsPowerUpOn;
									on_button_backing_pwrUp.GetComponent<SpriteRenderer>().isVisible = IsPowerUpOn;

									off_button_pwrUp.GetComponent<SpriteRenderer>().isVisible = !IsPowerUpOn;
									off_button_backing_pwrUp.GetComponent<SpriteRenderer>().isVisible = !IsPowerUpOn;

									// Set persistent system
									string powerUpsString = IsPowerUpOn.ToString();
									PRS.SetValue("powerUps", powerUpsString);
									if (scenelogic != null)
									{
										scenelogic.GetComponent<SceneLogic>().KeepInv(IsPowerUpOn);
									}

                                    playUISound = true;

								}
								else if (IS.GetKeyPress(InputKeys.D) || IS.GetKeyPress(InputKeys.Right) || ControllerInput(MenuNavigation.RIGHT))
								{
									//off
									IsPowerUpOn = false;
									off_button_pwrUp.GetComponent<SpriteRenderer>().isVisible = !IsPowerUpOn;
									off_button_backing_pwrUp.GetComponent<SpriteRenderer>().isVisible = !IsPowerUpOn;

									on_button_pwrUp.GetComponent<SpriteRenderer>().isVisible = IsPowerUpOn;
									on_button_backing_pwrUp.GetComponent<SpriteRenderer>().isVisible = IsPowerUpOn;
									// Set persistent system
									string powerUpsString = IsPowerUpOn.ToString();
									PRS.SetValue("powerUps", powerUpsString);
									if (scenelogic != null)
									{
										scenelogic.GetComponent<SceneLogic>().KeepInv(IsPowerUpOn);
									}

                                    playUISound = true;
								}
								break;
							case 1:
								if (IS.GetKeyPress(InputKeys.A) || IS.GetKeyPress(InputKeys.Left) || ControllerInput(MenuNavigation.LEFT))
								{
									//on
									IsInvulnerabilityOn = true;
									on_button_inv.GetComponent<SpriteRenderer>().isVisible = IsInvulnerabilityOn;
									on_button_backing_inv.GetComponent<SpriteRenderer>().isVisible = IsInvulnerabilityOn;

									off_button_inv.GetComponent<SpriteRenderer>().isVisible = !IsInvulnerabilityOn;
									off_button_backing_inv.GetComponent<SpriteRenderer>().isVisible = !IsInvulnerabilityOn;

									// Set persistent system
									string invulnerabilityString = IsInvulnerabilityOn.ToString();
									PRS.SetValue("invulnerability", invulnerabilityString);

									if (scenelogic != null)
									{
										scenelogic.GetComponent<SceneLogic>().CreativeMode(IsInvulnerabilityOn);
									}

                                    playUISound = true;
								}
								else if (IS.GetKeyPress(InputKeys.D) || IS.GetKeyPress(InputKeys.Right) || ControllerInput(MenuNavigation.RIGHT))
								{
									//off
									IsInvulnerabilityOn = false;
									off_button_inv.GetComponent<SpriteRenderer>().isVisible = !IsInvulnerabilityOn;
									off_button_backing_inv.GetComponent<SpriteRenderer>().isVisible = !IsInvulnerabilityOn;

									on_button_inv.GetComponent<SpriteRenderer>().isVisible = IsInvulnerabilityOn;
									on_button_backing_inv.GetComponent<SpriteRenderer>().isVisible = IsInvulnerabilityOn;

									// Set persistent system
									string invulnerabilityString = IsInvulnerabilityOn.ToString();
									PRS.SetValue("invulnerability", invulnerabilityString);

									if (scenelogic != null)
									{
										scenelogic.GetComponent<SceneLogic>().CreativeMode(IsInvulnerabilityOn);
									}

                                    playUISound = true;
								}
								break;
						}

						//move pointer
						switch (mCurrentEditMember)
						{
							case 0:
								settingsPointerTransform.Position = power_ups_panel.GetComponent<Transform>().Position;
								break;
							case 1:
								settingsPointerTransform.Position = invulnerability_panel.GetComponent<Transform>().Position;
								break;
						}
					}

					if ((IS.GetKeyPress(InputKeys.W) && !settingsPointer.GetComponent<SpriteRenderer>().isVisible) || ControllerInput(MenuNavigation.START))
					{

						//Debug.Log("Triggered ESC to not editing settings");
						mIsEditingSettings = false; //Set to not editing any option
						menustate = 1;

						//Set every pointer back to false
						settingsPointer.GetComponent<SpriteRenderer>().isVisible = false;
						controlsPointer.GetComponent<SpriteRenderer>().isVisible = false;

                        playUISound = true;
					}
				}
				else // confirmation menu logic which is menustate == 2
				{
					if (IS.GetKeyPress(InputKeys.A) || ControllerInput(MenuNavigation.LEFT))
					{
						if (menuOption == 0)
							menuOption = 1;
						else if (menuOption == 1)
							menuOption = 0;

                        playUISound = true;
					}

					if (IS.GetKeyPress(InputKeys.D) || ControllerInput(MenuNavigation.RIGHT))
					{
						if (menuOption == 0)
							menuOption = 1;
						else if (menuOption == 1)
							menuOption = 0;

                        playUISound = true;
					}

					if (IS.GetKeyPress(InputKeys.Enter) || ControllerInput(MenuNavigation.CONFIRM))
					{
						if (menuOption == 0) // yes
						{
							// go back to the main menu
							if (CurrentScene == "MainMenu")
							{
								Game.CloseGame();
							}

							else
							{
								Scene.TransitionScene("MainMenu", 4.5f);
							}

						}
						else if (menuOption == 1) // no
						{
							isConfirming = false;
							menustate = 0;
							isChangeMenu = true;
							menuOption = 1;
						}

                        playUISound = true;
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
							UIS.SetVisible(pauseMenu.ID, true);
							UIS.SetVisible(cfmMenu.ID, false);
							for (int i = 0; i < options.Count; i++)
								UIS.SetVisible(options[i].ID, true);
							for (int i = 0; i < DestructiveActionConfirmations.Count; i++)
								UIS.SetVisible(DestructiveActionConfirmations[i].ID, false);
							// show the pointer
							UIS.SetVisible(pointer.ID, true);
							UIS.SetVisible(destructivePointer.ID, false);
							break;
						case 1:
							//show settings menu
							UIS.SetVisible(settingsMenu.ID, true);
							UIS.SetVisible(settingsClose.ID, true);

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
							UIS.SetVisible(cfmMenu.ID, true);
							for (int i = 0; i < DestructiveActionConfirmations.Count; i++)
							{
								UIS.SetVisible(DestructiveActionConfirmations[i].ID, true);
							}
							// show the pointer
							UIS.SetVisible(destructivePointer.ID, true);
							break;
					}
					isChangeMenu = false;
				}
			}
			else if (isChangeMenu && !isPaused) //This is when you are getting out of the pause menu
			{
				// hide the whole pause menu
				UIS.SetVisible(pauseMenu.ID, false);
				UIS.SetVisible(cfmMenu.ID, false);

				// hide the pointer
				UIS.SetVisible(pointer.ID, false);
				UIS.SetVisible(destructivePointer.ID, false);

				for (int i = 0; i < options.Count; i++)
					UIS.SetVisible(options[i].ID, false);

				for (int i = 0; i < DestructiveActionConfirmations.Count; i++)
					UIS.SetVisible(DestructiveActionConfirmations[i].ID, false);

				UIS.SetVisible(settingsMenu.ID, false);
				UIS.SetVisible(settingsClose.ID, false);
				for (int i = 0; i < settingsPanel.Count; i++)
				{
					UIS.SetVisible(settingsPanel[i].ID, false);
					UIS.SetVisible(settingsSelected[i].ID, false);
				}

				for (int i = 0; i < audioPanel.Count; ++i)
				{
					if (audioPanel[i].HasComponent<Text>())
						TS.SetVisible(audioPanel[i].ID, false);

					else
						UIS.SetVisible(audioPanel[i].ID, false);
				}

				UIS.SetVisible(mGammaPanel.ID, false);
				TS.SetVisible(mGammaValue.ID, false);

				isChangeMenu = false;
			}


			// play UI sound here
            if (playUISound)
            {
                if (ECSManager.IsValidEntity(sfx))
                    AS.Play(sfx);

                playUISound = false;
            }
		}

		private bool ControllerInput(MenuNavigation button)
		{
			float leftStickY_0 = IS.GetControllerStickY(0, false);
			float leftStickY_1 = IS.GetControllerStickY(1, false);

			// Check if the controller input is valid
			switch (button)
			{
				case MenuNavigation.UP:
					if (IS.GetControllerButtonTriggered(0, IS.Button.DPadUp) || IS.GetControllerButtonTriggered(1, IS.Button.DPadUp))
						return true;
					// handle analog stick
					if (leftStickY_0 > 0.5f || leftStickY_1 > 0.5f)
						return true;
					break;
				case MenuNavigation.DOWN:
					if (IS.GetControllerButtonTriggered(0, IS.Button.DPadDown) || IS.GetControllerButtonTriggered(1, IS.Button.DPadDown))
						return true;
					// handle analog stick
					if (leftStickY_0 < -0.5f || leftStickY_1 < -0.5f)
						return true;
					break;
				case MenuNavigation.LEFT:
					if (IS.GetControllerButtonTriggered(0, IS.Button.DPadLeft) || IS.GetControllerButtonTriggered(1, IS.Button.DPadLeft))
						return true;
					// handle analog stick
					if (IS.GetControllerStickX(0, false) < -0.5f || IS.GetControllerStickX(1, false) < -0.5f)
						return true;
					break;
				case MenuNavigation.RIGHT:
					if (IS.GetControllerButtonTriggered(0, IS.Button.DPadRight) || IS.GetControllerButtonTriggered(1, IS.Button.DPadRight))
						return true;
					// handle analog stick
					if (IS.GetControllerStickX(0, false) > 0.5f || IS.GetControllerStickX(1, false) > 0.5f)
						return true;
					break;
				case MenuNavigation.CONFIRM:
					if (IS.GetControllerButtonTriggered(0, IS.Button.A) || IS.GetControllerButtonTriggered(1, IS.Button.A))
						return true;
					break;
				case MenuNavigation.BACK:
					if (IS.GetControllerButtonTriggered(0, IS.Button.B) || IS.GetControllerButtonTriggered(1, IS.Button.B))
						return true;
					break;
				case MenuNavigation.START:
					if (IS.GetControllerButtonTriggered(0, IS.Button.Start) || IS.GetControllerButtonTriggered(1, IS.Button.Start))
						return true;
					break;
				case MenuNavigation.SELECT:
					if (IS.GetControllerButtonTriggered(0, IS.Button.Back) || IS.GetControllerButtonTriggered(1, IS.Button.Back))
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
			settingsOption = 0;

			for (int x = 0; x < settingsSelected.Count; x++)
				UIS.SetVisible(settingsSelected[x].ID, false);

			for (int x = 0; x < settingsPanel.Count; x++)
				UIS.SetVisible(settingsPanel[x].ID, false);

			UIS.SetVisible(settingsMenu.ID, false);
			UIS.SetVisible(settingsPointer.ID, false);
			UIS.SetVisible(settingsClose.ID, false);

			if (showGraphicsPanel)
			{
				UIS.SetVisible(mGammaPanel.ID, false);
				TS.SetVisible(mGammaValue.ID, false);
				showGraphicsPanel = false;
			}

			if (showAudioPanel)
			{
				for (int x = 0; x < audioPanel.Count; x++)
				{
					if (audioPanel[x].HasComponent<Text>())
						TS.SetVisible(audioPanel[x].ID, false);
					UIS.SetVisible(audioPanel[x].ID, false);
				}

				showAudioPanel = false;
			}

			if (showControlsPanel)
			{
				UIS.SetVisible(controlsPointer.ID, false);
				UIS.SetVisible(P1_panel.ID, false);
				UIS.SetVisible(P1_keyboard_panel.ID, false);
				UIS.SetVisible(P1_keyboard_preset1.ID, false);
				UIS.SetVisible(P1_keyboard_preset2.ID, false);
				UIS.SetVisible(P1_keyboard_preset3.ID, false);
				UIS.SetVisible(P1_controller_panel.ID, false);
				UIS.SetVisible(P1_controller_preset1.ID, false);
				UIS.SetVisible(P1_controller_preset2.ID, false);
				UIS.SetVisible(P2_panel.ID, false);
				UIS.SetVisible(P2_keyboard_panel.ID, false);
				UIS.SetVisible(P2_keyboard_preset1.ID, false);
				UIS.SetVisible(P2_keyboard_preset2.ID, false);
				UIS.SetVisible(P2_keyboard_preset3.ID, false);
				UIS.SetVisible(P2_controller_panel.ID, false);
				UIS.SetVisible(P2_controller_preset1.ID, false);
				UIS.SetVisible(P2_controller_preset2.ID, false);
				showControlsPanel = false;
			}

			if (showGameplayPanel)
			{
				showGameplayPanel = false;
				UIS.SetVisible(power_ups_panel.ID, false);
				UIS.SetVisible(on_button_pwrUp.ID, false);
				UIS.SetVisible(on_button_backing_pwrUp.ID, false);
				UIS.SetVisible(off_button_pwrUp.ID, false);
				UIS.SetVisible(off_button_backing_pwrUp.ID, false);
				UIS.SetVisible(invulnerability_panel.ID, false);
				UIS.SetVisible(on_button_inv.ID, false);
				UIS.SetVisible(on_button_backing_inv.ID, false);
				UIS.SetVisible(off_button_inv.ID, false);
				UIS.SetVisible(off_button_backing_inv.ID, false);
			}
		}

		private void showControlPanel()
		{
			if (P1_IsKeyboard)
			{
				P1_keyboard_panel.GetComponent<SpriteRenderer>().isVisible = showControlsPanel;
				P1_controller_panel.GetComponent<SpriteRenderer>().isVisible = false;

				switch (P1_KeyboardPreset)
				{
					case 0:
						P1_keyboard_preset1.GetComponent<SpriteRenderer>().isVisible = showControlsPanel;
						P1_keyboard_preset2.GetComponent<SpriteRenderer>().isVisible = false;
						P1_keyboard_preset3.GetComponent<SpriteRenderer>().isVisible = false;
						P1_controller_preset1.GetComponent<SpriteRenderer>().isVisible = false;
						P1_controller_preset2.GetComponent<SpriteRenderer>().isVisible = false;
						break;
					case 1:
						P1_keyboard_preset1.GetComponent<SpriteRenderer>().isVisible = false;
						P1_keyboard_preset2.GetComponent<SpriteRenderer>().isVisible = showControlsPanel;
						P1_keyboard_preset3.GetComponent<SpriteRenderer>().isVisible = false;
						P1_controller_preset1.GetComponent<SpriteRenderer>().isVisible = false;
						P1_controller_preset2.GetComponent<SpriteRenderer>().isVisible = false;
						break;
					case 2:
						P1_keyboard_preset1.GetComponent<SpriteRenderer>().isVisible = false;
						P1_keyboard_preset2.GetComponent<SpriteRenderer>().isVisible = false;
						P1_keyboard_preset3.GetComponent<SpriteRenderer>().isVisible = showControlsPanel;
						P1_controller_preset1.GetComponent<SpriteRenderer>().isVisible = false;
						P1_controller_preset2.GetComponent<SpriteRenderer>().isVisible = false;

						break;
				}
			}
			else
			{
				P1_keyboard_panel.GetComponent<SpriteRenderer>().isVisible = false;
				P1_controller_panel.GetComponent<SpriteRenderer>().isVisible = showControlsPanel;

				if (P1_ControllerPreset == 0)
				{
					P1_keyboard_preset1.GetComponent<SpriteRenderer>().isVisible = false;
					P1_keyboard_preset2.GetComponent<SpriteRenderer>().isVisible = false;
					P1_keyboard_preset3.GetComponent<SpriteRenderer>().isVisible = false;
					P1_controller_preset1.GetComponent<SpriteRenderer>().isVisible = showControlsPanel;
					P1_controller_preset2.GetComponent<SpriteRenderer>().isVisible = false;
				}
				else if (P1_ControllerPreset == 1)
				{
					P1_keyboard_preset1.GetComponent<SpriteRenderer>().isVisible = false;
					P1_keyboard_preset2.GetComponent<SpriteRenderer>().isVisible = false;
					P1_keyboard_preset3.GetComponent<SpriteRenderer>().isVisible = false;
					P1_controller_preset1.GetComponent<SpriteRenderer>().isVisible = false;
					P1_controller_preset2.GetComponent<SpriteRenderer>().isVisible = showControlsPanel;
				}
			}

			if (P2_IsKeyboard)
			{
				P2_keyboard_panel.GetComponent<SpriteRenderer>().isVisible = showControlsPanel;
				P2_controller_panel.GetComponent<SpriteRenderer>().isVisible = false;

				switch (P2_KeyboardPreset)
				{
					case 0:
						P2_keyboard_preset1.GetComponent<SpriteRenderer>().isVisible = showControlsPanel;
						P2_keyboard_preset2.GetComponent<SpriteRenderer>().isVisible = false;
						P2_keyboard_preset3.GetComponent<SpriteRenderer>().isVisible = false;
						P2_controller_preset1.GetComponent<SpriteRenderer>().isVisible = false;
						P2_controller_preset2.GetComponent<SpriteRenderer>().isVisible = false;
						break;
					case 1:
						P2_keyboard_preset1.GetComponent<SpriteRenderer>().isVisible = false;
						P2_keyboard_preset2.GetComponent<SpriteRenderer>().isVisible = showControlsPanel;
						P2_keyboard_preset3.GetComponent<SpriteRenderer>().isVisible = false;
						P2_controller_preset1.GetComponent<SpriteRenderer>().isVisible = false;
						P2_controller_preset2.GetComponent<SpriteRenderer>().isVisible = false;
						break;
					case 2:
						P2_keyboard_preset1.GetComponent<SpriteRenderer>().isVisible = false;
						P2_keyboard_preset2.GetComponent<SpriteRenderer>().isVisible = false;
						P2_keyboard_preset3.GetComponent<SpriteRenderer>().isVisible = showControlsPanel;
						P2_controller_preset1.GetComponent<SpriteRenderer>().isVisible = false;
						P2_controller_preset2.GetComponent<SpriteRenderer>().isVisible = false;
						break;
				}
			}
			else
			{
				P2_keyboard_panel.GetComponent<SpriteRenderer>().isVisible = false;
				P2_controller_panel.GetComponent<SpriteRenderer>().isVisible = showControlsPanel;

				if (P2_ControllerPreset == 0)
				{
					P2_keyboard_preset1.GetComponent<SpriteRenderer>().isVisible = false;
					P2_keyboard_preset2.GetComponent<SpriteRenderer>().isVisible = false;
					P2_keyboard_preset3.GetComponent<SpriteRenderer>().isVisible = false;
					P2_controller_preset1.GetComponent<SpriteRenderer>().isVisible = showControlsPanel;
					P2_controller_preset2.GetComponent<SpriteRenderer>().isVisible = false;
				}
				else if (P2_ControllerPreset == 1)
				{
					P2_keyboard_preset1.GetComponent<SpriteRenderer>().isVisible = false;
					P2_keyboard_preset2.GetComponent<SpriteRenderer>().isVisible = false;
					P2_keyboard_preset3.GetComponent<SpriteRenderer>().isVisible = false;
					P2_controller_preset1.GetComponent<SpriteRenderer>().isVisible = false;
					P2_controller_preset2.GetComponent<SpriteRenderer>().isVisible = showControlsPanel;
				}
			}
		}

		/// <summary>Allows other classes and functions to trigger the pause menu.</summary>
		public void SendExternalPauseCommand() => externalPauseCommand = true;

		/// <summary>Returns whether an external pause command was sent to this class and
		/// immediately resets the flag's value(like a postfix operator).</summary>
		/// <returns>Whether an external pause command was sent to this class.</returns>
		private bool ReadExternalPauseCommand()
		{
			bool oldValue = externalPauseCommand;
			externalPauseCommand = false;
			return oldValue;
		}
	}
}
