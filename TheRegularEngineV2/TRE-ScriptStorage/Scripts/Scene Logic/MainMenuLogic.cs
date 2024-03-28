using System;
using GlmSharp;

namespace TRE
{
	using IS = InputSystem;
	using PS = PhysicsSystem;

	public class MainMenuLogic : Entity
	{
		Entity UIControls;
		Entity Moley;
		Entity Holey;

		Entity ToOptionSelect;
		Entity ToLevelSelect;
		Entity ToCreditsSelect;
		Entity ToQuitSelect;

		Entity QuitConfirmation;
		Entity QuitConfirmationYes;
		Entity QuitConfirmationNo;

		Entity QuitYesNoPointer;

		bool PopupQuitConfirmation = false;
		bool CurrentButtonSelected = false;
		bool selectedOption = false;
		bool selectedLevel = false;
		bool selectedCredits = false;
		bool selectedQuit = false;

		// Level Select
		Entity ToReturnSelect;
		Entity ToTutorialSelect;
		Entity ToLevel1Select;
		Entity ToLevel2Select;

		bool selectedReturn = false;
		bool selectedTutorial = false;
		bool selectedLevel1 = false;
		bool selectedLevel2 = false;

		// Controller UI
		bool controller1 = false;
		bool lastController1 = false;
		bool controller2 = false;
		bool lastController2 = false;
		bool changeUI = false;

		const float sceneTransitionDelay = 2.5f;

		// Timer for animation
		float currentTimer = 0;
		const float delayJumpingHole = 0.68f;

		/// <summary>An array of titles to manage in the Main Menu scene. Assigned in Start().</summary>
		Entity[] titleList;

		Entity TitleStarsCollected;
		Entity Stars1;
		Entity Stars2;
		Entity Stars3;

		const float hiddenYPos = -800f;
		const float displayYPos = -400f;
		const float titleMoveSpeed = 10f;
		const float titleOffset = 0.05f;

		public void Start()
		{
			UIControls = ECSManager.FindEntityByName("Controls_UI");
			Moley = ECSManager.FindEntityByName("Moley");
			Holey = ECSManager.FindEntityByName("Holey");

			ToOptionSelect = ECSManager.FindEntityByName("ToOptionSelect");
			ToLevelSelect = ECSManager.FindEntityByName("ToLevelSelect");
			ToCreditsSelect = ECSManager.FindEntityByName("ToCreditsSelect");
			ToQuitSelect = ECSManager.FindEntityByName("ToQuitSelect");

			ToReturnSelect = ECSManager.FindEntityByName("ToReturnSelect");
			ToTutorialSelect = ECSManager.FindEntityByName("ToTutorialSelect");
			ToLevel1Select = ECSManager.FindEntityByName("ToLevel1Select");
			ToLevel2Select = ECSManager.FindEntityByName("ToLevel2Select");

			QuitConfirmation = ECSManager.FindEntityByName("QuitConfirmationPopup");
			QuitConfirmationYes = ECSManager.FindEntityByName("YesQuit");
			QuitConfirmationNo = ECSManager.FindEntityByName("NoQuit");
			QuitYesNoPointer = ECSManager.FindEntityByName("CurrentButtonPointer");

			titleList = new Entity[]
			{
				ECSManager.FindEntityByName("TitleLevelSelect"),
				ECSManager.FindEntityByName("TitleQuitGame"),
				ECSManager.FindEntityByName("TitleTutorial"),
				ECSManager.FindEntityByName("TitleLevel1"),
				ECSManager.FindEntityByName("TitleLevel2")
			};

			TitleStarsCollected = ECSManager.FindEntityByName("TitleStarsCollected");
			Stars1 = ECSManager.FindEntityByName("Star1");
			Stars2 = ECSManager.FindEntityByName("Star2");
			Stars3 = ECSManager.FindEntityByName("Star3");

			foreach (Entity title in titleList)
				title.GetComponent<TextBounce>().Pause();

			TitleStarsCollected.GetComponent<TextBounce>().Pause();

			// Determine where to spawn the moles based on previous scene
			string prevScene = PersistentSystem.GetValue("PrevScene");
			//Debug.Log("START " + prevScene);
			if (prevScene == "Tutorial")
			{
				// Teleport Moley and Holey to another location
				vec3 teleportPos = ToTutorialSelect.GetComponent<Transform>().Position;
				Moley.GetComponent<Transform>().Position = new vec3(teleportPos.x - 5, teleportPos.y + 15, teleportPos.z);
				Holey.GetComponent<Transform>().Position = new vec3(teleportPos.x + 5, teleportPos.y + 15, teleportPos.z);

				JumpOutHole();
			}
			else if (prevScene == "Level_1")
			{
				// Teleport Moley and Holey to another location
				vec3 teleportPos = ToLevel1Select.GetComponent<Transform>().Position;
				Moley.GetComponent<Transform>().Position = new vec3(teleportPos.x - 5, teleportPos.y + 15, teleportPos.z);
				Holey.GetComponent<Transform>().Position = new vec3(teleportPos.x + 5, teleportPos.y + 15, teleportPos.z);

				JumpOutHole();
			}
			else if (prevScene == "Level_2")
			{
				// Teleport Moley and Holey to another location
				vec3 teleportPos = ToLevel2Select.GetComponent<Transform>().Position;
				Moley.GetComponent<Transform>().Position = new vec3(teleportPos.x - 5, teleportPos.y + 15, teleportPos.z);
				Holey.GetComponent<Transform>().Position = new vec3(teleportPos.x + 5, teleportPos.y + 15, teleportPos.z);

				JumpOutHole();
			}
			else if (prevScene == "Credits_Scene")
			{
				// Teleport Moley and Holey to another location
				vec3 teleportPos = ToCreditsSelect.GetComponent<Transform>().Position;
				Moley.GetComponent<Transform>().Position = new vec3(teleportPos.x - 5, teleportPos.y + 15, teleportPos.z);
				Holey.GetComponent<Transform>().Position = new vec3(teleportPos.x + 5, teleportPos.y + 15, teleportPos.z);

				JumpOutHole();
			}
		}

		public void Update()
		{
			#region Controller UI
			controller1 = IS.GetControllerConnected(0);
			controller2 = IS.GetControllerConnected(1);

			// Controller check
			if (controller1)
			{
				if (lastController1 == false)
				{
					lastController1 = true;
					changeUI = true;
				}
			}
			else
			{
				if (lastController1 == false) { }
				else
				{
					lastController1 = false;
					changeUI = true;
				}
			}

			if (controller2)
			{
				if (lastController2 == false)
				{
					lastController2 = true;
					changeUI = true;
				}
			}
			else
			{
				if (lastController2 == false) { }
				else
				{
					lastController2 = false;
					changeUI = true;
				}

			}

			//Controller UI 
			if (controller1 && !controller2 && changeUI)
			{
				UIControls.GetComponent<SpriteRenderer>().Texture = "ui-mm-controls-controller1.png";
				controller1 = false;
			}
			else if (controller1 && controller2 && changeUI)
			{
				UIControls.GetComponent<SpriteRenderer>().Texture = "ui-mm-controls-controller2.png";
				controller1 = false;
			}
			else if (!controller1 && !controller2 && changeUI)
			{
				UIControls.GetComponent<SpriteRenderer>().Texture = "ui-mm-controls.png";
				changeUI = false;
			}

			// UIControls: Determines if fade in or out
			if (UIControls != null && Moley != null && Holey != null)
			{
				bool MoleyMoving = Moley.GetComponent<MoleyController>().isJumping || Moley.GetComponent<MoleyController>().isWalking;
				bool HoleyMoving = Holey.GetComponent<HoleyController>().isJumping || Holey.GetComponent<HoleyController>().isWalking;

				// UIControls is visible and Moley and Holey are moving
				if (UIControls.GetComponent<SpriteRenderer>().Color.w > 0 && MoleyMoving && HoleyMoving)
				{
					UIControls.GetComponent<VFX_FadeIn>().ForceStop();
					UIControls.GetComponent<VFX_FadeOut>().FadeOut();
				}
				else if (UIControls.GetComponent<SpriteRenderer>().Color.w < 1 && !MoleyMoving && !HoleyMoving)
				{
					UIControls.GetComponent<VFX_FadeOut>().ForceStop();
					UIControls.GetComponent<VFX_FadeIn>().FadeIn();
				}
			}
			#endregion

			#region TitleLerpLogic
			// Determine which title needs to be displayed.
			vec3[] titlePositions = new vec3[titleList.Length];
			for (int i = 0; i < titleList.Length; ++i)
				titlePositions[i] = titleList[i].GetComponent<Transform>().Position;

			// initialized with an invalid value
			int titleToDisplayIndex = -1;

			if (ToLevelSelect.GetComponent<TunnelLogic>().MolesInside() || ToReturnSelect.GetComponent<TunnelLogic>().MolesInside())
				titleToDisplayIndex = 0;
			else if (ToQuitSelect.GetComponent<TunnelLogic>().MolesInside())
				titleToDisplayIndex = 1;
			else if (ToTutorialSelect.GetComponent<TunnelLogic>().MolesInside())
				titleToDisplayIndex = 2;
			else if (ToLevel1Select.GetComponent<TunnelLogic>().MolesInside())
				titleToDisplayIndex = 3;
			else if (ToLevel2Select.GetComponent<TunnelLogic>().MolesInside())
				titleToDisplayIndex = 4;

			// If there is a title to display, display it.
			// Otherwise, hide it and pause its bouncing.
			for (int i = 0; i < titleList.Length; ++i)
			{
				if (i != titleToDisplayIndex)
				{
					titlePositions[i].y = hiddenYPos;
					titleList[i].GetComponent<TextBounce>().Pause();
				}
				else
					titlePositions[i].y = displayYPos;
			}

			// Lerp title to pos.
			float[] titlePositionsY = new float[titleList.Length];
			for (int i = 0; i < titleList.Length; ++i)
				titlePositionsY[i] = MathF.Lerp(titleList[i].GetComponent<Transform>().Position.y, titlePositions[i].y, titleMoveSpeed * Time.deltaTime);

			// Set title's new position (with the lerped position above).
			for (int i = 0; i < titleList.Length; ++i)
				titleList[i].GetComponent<Transform>().Position = new vec3(titlePositions[i].x, titlePositionsY[i], titlePositions[i].z);

			// Resume Text Bounce if it is close to the position.
			for (int i = 0; i < titleList.Length; ++i)
				if (titleList[i].GetComponent<TextBounce>().IsPaused() && titlePositions[i].y == displayYPos && Math.Abs(titlePositionsY[i] - titlePositions[i].y) < titleOffset)
					titleList[i].GetComponent<TextBounce>().Resume();
			#endregion

			#region Stars
			// Do for stars collected
			vec3 titleStarsCollectedPos = TitleStarsCollected.GetComponent<Transform>().Position;
			if (ToTutorialSelect.GetComponent<TunnelLogic>().MolesInside() || ToLevel1Select.GetComponent<TunnelLogic>().MolesInside() || ToLevel2Select.GetComponent<TunnelLogic>().MolesInside())
			{
				titleStarsCollectedPos.y = displayYPos;
				// Determine which stars to display
				DetermineStarsDisplay();
			}
			else
			{
				titleStarsCollectedPos.y = hiddenYPos;
				TitleStarsCollected.GetComponent<TextBounce>().Pause();
			}
			// Lerp title to pos
			float titleStarsCollectedPosY = MathF.Lerp(TitleStarsCollected.GetComponent<Transform>().Position.y, titleStarsCollectedPos.y, titleMoveSpeed * Time.deltaTime);
			TitleStarsCollected.GetComponent<Transform>().Position = new vec3(titleStarsCollectedPos.x, titleStarsCollectedPosY, titleStarsCollectedPos.z);
			// Resume Text Bounce if it is close to the position
			if (TitleStarsCollected.GetComponent<TextBounce>().IsPaused() && titleStarsCollectedPos.y == displayYPos && Math.Abs(titleStarsCollectedPosY - titleStarsCollectedPos.y) < titleOffset)
			{
				TitleStarsCollected.GetComponent<TextBounce>().Resume();
			}
			#endregion

			bool isAnythingSelected = selectedOption || selectedLevel || selectedCredits || selectedQuit ||
				selectedReturn || selectedTutorial || selectedLevel1 || selectedLevel2;

			// Determine what tunnels were approved by both moles
			if (!isAnythingSelected && currentTimer <= 0)
			{
				if (ToOptionSelect.GetComponent<TunnelLogic>().MolesApproved() && !selectedOption)
				{
					//selectedOption = true;
					//JumpIntoHole();
					//ToOptionSelect.GetComponent<TunnelLogic>().ResetMoles();

					// SHOW OPTIONS
				}
				else if (ToLevelSelect.GetComponent<TunnelLogic>().MolesApproved() && !selectedLevel)
				{
					selectedLevel = true;
					JumpIntoHole();
					ToLevelSelect.GetComponent<TunnelLogic>().ResetMoles();
				}
				else if (ToCreditsSelect.GetComponent<TunnelLogic>().MolesApproved() && !selectedCredits)
				{
					selectedCredits = true;
					JumpIntoHole();
					ToCreditsSelect.GetComponent<TunnelLogic>().ResetMoles();
				}
				else if (ToQuitSelect.GetComponent<TunnelLogic>().MolesApproved() && !selectedQuit)
				{
					selectedQuit = true;
					JumpIntoHole();
					ToQuitSelect.GetComponent<TunnelLogic>().ResetMoles();
				}
				else if (ToReturnSelect.GetComponent<TunnelLogic>().MolesApproved() && !selectedReturn)
				{
					selectedReturn = true;
					JumpIntoHole();
					ToReturnSelect.GetComponent<TunnelLogic>().ResetMoles();
				}
				else if (ToTutorialSelect.GetComponent<TunnelLogic>().MolesApproved() && !selectedTutorial)
				{
					selectedTutorial = true;
					JumpIntoHole();
					ToTutorialSelect.GetComponent<TunnelLogic>().ResetMoles();
				}
				else if (ToLevel1Select.GetComponent<TunnelLogic>().MolesApproved() && !selectedLevel1)
				{
					selectedLevel1 = true;
					JumpIntoHole();
					ToLevel1Select.GetComponent<TunnelLogic>().ResetMoles();
				}
				else if (ToLevel2Select.GetComponent<TunnelLogic>().MolesApproved() && !selectedLevel2)
				{
					selectedLevel2 = true;
					JumpIntoHole();
					ToLevel2Select.GetComponent<TunnelLogic>().ResetMoles();
				}
			}

			if (currentTimer > 0) currentTimer -= Time.deltaTime;

			isAnythingSelected = selectedOption || selectedLevel || selectedCredits || selectedQuit ||
				selectedReturn || selectedTutorial || selectedLevel1 || selectedLevel2;

			// Determine action of selection
			if (currentTimer <= 0 && isAnythingSelected)
			{
				currentTimer = 0;

				PS.SetLinearVelocity(Moley.ID, vec3.Zero);
				PS.SetLinearVelocity(Holey.ID, vec3.Zero);

				// Force Holey and Moley to stop dropping to do stuff
				Moley.GetComponent<Rigidbody>().useGravity = false;
				Holey.GetComponent<Rigidbody>().useGravity = false;

				if (selectedOption)
				{
					selectedOption = false;
				}
				else if (selectedLevel)
				{
					selectedLevel = false;

					// Teleport Moley and Holey to another location
					vec3 teleportPos = ToReturnSelect.GetComponent<Transform>().Position;
					Moley.GetComponent<Transform>().Position = new vec3(teleportPos.x - 5, teleportPos.y + 15, teleportPos.z);
					Holey.GetComponent<Transform>().Position = new vec3(teleportPos.x + 5, teleportPos.y + 15, teleportPos.z);

					JumpOutHole();
				}
				else if (selectedQuit)
				{
					selectedQuit = false;
					PopupQuitConfirmation = true;
					QuitConfirmation.GetComponent<SpriteRenderer>().isVisible = true;
					QuitConfirmationYes.GetComponent<SpriteRenderer>().isVisible = true;
					QuitConfirmationNo.GetComponent<SpriteRenderer>().isVisible = true;
					QuitYesNoPointer.GetComponent<SpriteRenderer>().isVisible = true;
				}
				else if (selectedReturn)
				{
					selectedReturn = false;

					// Teleport Moley and Holey to another location
					vec3 teleportPos = ToLevelSelect.GetComponent<Transform>().Position;
					Moley.GetComponent<Transform>().Position = new vec3(teleportPos.x - 5, teleportPos.y + 25, teleportPos.z);
					Holey.GetComponent<Transform>().Position = new vec3(teleportPos.x + 5, teleportPos.y + 25, teleportPos.z);

					JumpOutHole();
				}
				else if (selectedTutorial)
				{
					selectedTutorial = false;

					Scene.TransitionScene("CutsceneStart", sceneTransitionDelay);
				}
				else if (selectedLevel1)
				{
					selectedLevel1 = false;

					Scene.TransitionScene("Level_1", sceneTransitionDelay);
				}
				else if (selectedLevel2)
				{
					selectedLevel2 = false;

					Scene.TransitionScene("Level_2", sceneTransitionDelay);
				}
				else if (selectedCredits)
				{
					selectedCredits = false;

					Scene.TransitionScene("Credits_Scene", sceneTransitionDelay);
				}
			}

			if (PopupQuitConfirmation)
			{
				if (IS.GetKeyPress(InputKeys.Q) || (!CurrentButtonSelected && IS.GetKeyPress(InputKeys.Enter)))
				{
					CurrentButtonSelected = false;
					QuitConfirmation.GetComponent<SpriteRenderer>().isVisible = false;
					QuitConfirmationYes.GetComponent<SpriteRenderer>().isVisible = false;
					QuitConfirmationNo.GetComponent<SpriteRenderer>().isVisible = false;
					QuitYesNoPointer.GetComponent<SpriteRenderer>().isVisible = false;
					PopupQuitConfirmation = false;

					vec3 teleportPos = ToQuitSelect.GetComponent<Transform>().Position;
					Moley.GetComponent<Transform>().Position = new vec3(teleportPos.x - 5, teleportPos.y + 15, teleportPos.z);
					Holey.GetComponent<Transform>().Position = new vec3(teleportPos.x + 5, teleportPos.y + 15, teleportPos.z);
					JumpOutHole();
				}

				if (IS.GetKeyPress(InputKeys.A))
				{
					CurrentButtonSelected = true;
					QuitYesNoPointer.GetComponent<Transform>().Position = QuitConfirmationYes.GetComponent<Transform>().Position;
				}

				if (IS.GetKeyPress(InputKeys.D))
				{
					CurrentButtonSelected = false;
					QuitYesNoPointer.GetComponent<Transform>().Position = QuitConfirmationNo.GetComponent<Transform>().Position;
				}

				if (CurrentButtonSelected && IS.GetKeyPress(InputKeys.Enter))
				{
					Game.CloseGame();
				}
			}

			isAnythingSelected = selectedOption || selectedLevel || selectedCredits || selectedQuit ||
				selectedReturn || selectedTutorial || selectedLevel1 || selectedLevel2;

			// Return controls to moles if they land on the ground
			if (!isAnythingSelected)
			{
				if (!Moley.GetComponent<MoleyController>().isControllable && Moley.GetComponent<MoleyController>().isGrounded && !Moley.GetComponent<CapsuleCollider>().IsTrigger)
					Moley.GetComponent<MoleyController>().isControllable = true;

				if (!Holey.GetComponent<HoleyController>().isControllable && Holey.GetComponent<HoleyController>().isGrounded && !Holey.GetComponent<CapsuleCollider>().IsTrigger)
					Holey.GetComponent<HoleyController>().isControllable = true;
			}
		}

		private void JumpIntoHole()
		{
			if (currentTimer > 0) return;

			Moley.GetComponent<MoleyController>().isControllable = false;
			Holey.GetComponent<HoleyController>().isControllable = false;

			PS.GetLinearVelocity(Moley.ID, out vec3 MoleyVel);
			PS.GetLinearVelocity(Holey.ID, out vec3 HoleyVel);

			Moley.GetComponent<MoleyController>().jumpCancelled = true;
			Holey.GetComponent<HoleyController>().jumpCancelled = true;

			PS.SetLinearVelocity(Moley.ID, vec3.Zero);
			PS.SetLinearVelocity(Holey.ID, vec3.Zero);

			if (MoleyVel.y < 0 || !Moley.GetComponent<MoleyController>().isJumping)
				PS.SetLinearVelocity(Moley.ID, new vec3(0, 50, 0));

			if (HoleyVel.y < 0 || !Holey.GetComponent<HoleyController>().isJumping)
				PS.SetLinearVelocity(Holey.ID, new vec3(0, 50, 0));

			Moley.GetComponent<CapsuleCollider>().IsTrigger = true;
			Holey.GetComponent<CapsuleCollider>().IsTrigger = true;
			currentTimer = delayJumpingHole;
		}

		private void JumpOutHole()
		{
			if (currentTimer > 0) return;

			// Give controls back to Moley and Holey
			Moley.GetComponent<Rigidbody>().useGravity = true;
			Holey.GetComponent<Rigidbody>().useGravity = true;

			//Moley.GetComponent<MoleyController>().isControllable = true;
			//Holey.GetComponent<HoleyController>().isControllable = true;

			Moley.GetComponent<MoleyController>().isJumping = false;
			Holey.GetComponent<HoleyController>().isJumping = false;

			Moley.GetComponent<MoleyController>().jumpCancelled = true;
			Holey.GetComponent<HoleyController>().jumpCancelled = true;

			Moley.GetComponent<CapsuleCollider>().IsTrigger = false;
			Holey.GetComponent<CapsuleCollider>().IsTrigger = false;

			PS.SetLinearVelocity(Moley.ID, vec3.Zero);
			PS.SetLinearVelocity(Holey.ID, vec3.Zero);

			PS.SetLinearVelocity(Moley.ID, new vec3(0, 70, 0));
			PS.SetLinearVelocity(Holey.ID, new vec3(0, 70, 0));
		}

		private void DetermineStarsDisplay()
		{
			string holeSceneName = "";
			if (ToTutorialSelect.GetComponent<TunnelLogic>().MolesInside())
				holeSceneName = "Tutorial";
			else if (ToLevel1Select.GetComponent<TunnelLogic>().MolesInside())
				holeSceneName = "Level_1";
			else if (ToLevel2Select.GetComponent<TunnelLogic>().MolesInside())
				holeSceneName = "Level_2";

			Stars1.SetActive(false);
			Stars2.SetActive(false);
			Stars3.SetActive(false);

			if (int.TryParse(PersistentSystem.GetValue(holeSceneName + "StarsObtained"), out int mapStars))
			{
				//Debug.Log(holeSceneName + "StarsObtained: " + mapStars);
			}
			if (mapStars >= 3)
			{
				Stars3.SetActive(true);
			}
			if (mapStars >= 2)
			{
				Stars2.SetActive(true);
			}
			if (mapStars >= 1)
			{
				Stars1.SetActive(true);
			}
		}
	}
}