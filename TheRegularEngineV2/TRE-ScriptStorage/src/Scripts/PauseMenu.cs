using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using GlmSharp;

namespace TRE
{
	public class PauseMenu : Entity
	{
		public bool isPaused = false;
		public bool isConfirming = false;
		public bool isChangeMenu = false;

		public int mainMenuOption = 0; // for confirmation menu to know which option to execute
		public int currentOption = 0;
		public int menuOption = 1;
		public int menustate = 0; //0 = main menu, 1 = show controls,2 = confirmation menu



		// Pause menu pointer
		private Entity pointer;
		private Transform pointerTransform;

		private Entity destructivePointer;
		private Transform destructivePointerTransform;

		// Pause menu options
		private List<Entity> options;
		private List<Entity> DestructiveActionConfirmations;

		private Entity pauseMenu;
		private Entity cfmMenu;



		public void Start()
		{
			options = new List<Entity>();
			DestructiveActionConfirmations = new List<Entity>();

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
		}

		public void OnCreate()
		{

		}

		public void Update()
		{
			if (InputSystem.GetKeyPress(InputKeys.Escape))
			{
				isPaused = !isPaused;
				isChangeMenu = true;
				menustate = 0;
				currentOption = 0;
			}

			if (isPaused)
			{
				// Pause menu logic
				if (menustate == 0)
				{
					//we are assuming that pause menu entering is handled by game logic
					if (InputSystem.GetKeyPress(InputKeys.W))
					{
						if (currentOption == 0)
							currentOption = 2;
						else
							currentOption -= 1;
					}

					if (InputSystem.GetKeyPress(InputKeys.S))
					{
						if (currentOption == 2)
							currentOption = 0;
						else
							currentOption += 1;
					}

					if (InputSystem.GetKeyPress(InputKeys.Enter))
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
						else if (currentOption == 2) // Quit Gane
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

				}

				// confirmation menu logic
				else
				{
					if (InputSystem.GetKeyPress(InputKeys.A))
					{
						if (menuOption == 0)
							menuOption = 1;
						else if (menuOption == 1)
							menuOption = 0;
					}

					if (InputSystem.GetKeyPress(InputKeys.D))
					{
						if (menuOption == 0)
							menuOption = 1;
						else if (menuOption == 1)
							menuOption = 0;
					}

					if (InputSystem.GetKeyPress(InputKeys.Enter))
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

			else if(isChangeMenu && !isPaused)
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

		public bool IsPaused()
		{
			return isPaused;
		}
	}
}
