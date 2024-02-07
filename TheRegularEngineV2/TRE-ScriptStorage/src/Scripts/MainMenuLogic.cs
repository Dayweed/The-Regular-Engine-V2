using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using GlmSharp;
using System.Runtime.Remoting.Metadata.W3cXsd2001;

namespace TRE
{
	public class MainMenuLogic : Entity
	{
		Entity UIControls;
		Entity Moley;
		Entity Holey;

		Entity ToOptionSelect;
		Entity ToLevelSelect;
		Entity ToQuitSelect;
		Entity ToReturnSelect;

		bool selectedOption = false;
		bool selectedLevel = false;
		bool selectedQuit = false;
		bool selectedReturn = false;

		// Level Select
		Entity ToTutorialSelect;
		Entity ToLevel1Select;
		Entity ToLevel2Select;

		bool selectedTutorial = false;
		bool selectedLevel1 = false;
		bool selectedLevel2 = false;

		// Timer for animation
		float currentTimer = 0;
		float delayJumpingHole = 0.68f;

		// Title to display
		Entity TitleLevelSelect;
		Entity TitleQuitGame;
		Entity TitleStarsCollected;
		Entity Stars1;
		Entity Stars2;
		Entity Stars3;

		float hiddenYPos = -800f;
		float displayYPos = -400f;
		float titleMoveSpeed = 10f;
		float titleOffset = 0.05f;

		public void Start()
		{
			UIControls = ECSManager.FindEntityByName("Controls_UI");
			Moley = ECSManager.FindEntityByName("Moley");
			Holey = ECSManager.FindEntityByName("Holey");

			ToOptionSelect = ECSManager.FindEntityByName("ToOptionSelect");
			ToLevelSelect = ECSManager.FindEntityByName("ToLevelSelect");
			ToQuitSelect = ECSManager.FindEntityByName("ToQuitSelect");
			ToReturnSelect = ECSManager.FindEntityByName("ToReturnSelect");

			ToTutorialSelect = ECSManager.FindEntityByName("ToTutorialSelect");
			ToLevel1Select = ECSManager.FindEntityByName("ToLevel1Select");
			ToLevel2Select = ECSManager.FindEntityByName("ToLevel2Select");

			TitleLevelSelect = ECSManager.FindEntityByName("TitleLevelSelect");
			TitleQuitGame = ECSManager.FindEntityByName("TitleQuitGame");
			TitleStarsCollected = ECSManager.FindEntityByName("TitleStarsCollected");
            Stars1 = ECSManager.FindEntityByName("Star1");
            Stars2 = ECSManager.FindEntityByName("Star2");
            Stars3 = ECSManager.FindEntityByName("Star3");

			TitleLevelSelect.GetComponent<TextBounce>().Pause();
			TitleQuitGame.GetComponent<TextBounce>().Pause();
			TitleStarsCollected.GetComponent<TextBounce>().Pause();

            // Determine where to spawn the moles based on previous scene
            String prevScene = PersistentSystem.GetValue("PrevScene");
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
        }

		public void Update()
		{
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

			#region TitleLerpLogic
			// Title: Determine which title to show
			vec3 titleLevelSelectPos = TitleLevelSelect.GetComponent<Transform>().Position;
			vec3 titleQuitGamePos = TitleQuitGame.GetComponent<Transform>().Position;
			if (ToLevelSelect.GetComponent<TunnelLogic>().MolesInside() || ToTutorialSelect.GetComponent<TunnelLogic>().MolesInside() || ToLevel1Select.GetComponent<TunnelLogic>().MolesInside() || ToLevel2Select.GetComponent<TunnelLogic>().MolesInside())
			{
				titleLevelSelectPos.y = displayYPos;
				titleQuitGamePos.y = hiddenYPos;
				TitleQuitGame.GetComponent<TextBounce>().Pause();
			}
			else if (ToQuitSelect.GetComponent<TunnelLogic>().MolesInside())
			{
				titleLevelSelectPos.y = hiddenYPos;
				titleQuitGamePos.y = displayYPos;
				TitleLevelSelect.GetComponent<TextBounce>().Pause();
			}
			else
			{
				titleLevelSelectPos.y = hiddenYPos;
				titleQuitGamePos.y = hiddenYPos;
				TitleLevelSelect.GetComponent<TextBounce>().Pause();
				TitleQuitGame.GetComponent<TextBounce>().Pause();
			}
			// Lerp title to pos
			float titleLevelSelectPosY = MathF.Lerp(TitleLevelSelect.GetComponent<Transform>().Position.y, titleLevelSelectPos.y, titleMoveSpeed * Time.deltaTime);
			float titleQuitGamePosY = MathF.Lerp(TitleQuitGame.GetComponent<Transform>().Position.y, titleQuitGamePos.y, titleMoveSpeed * Time.deltaTime);
			TitleLevelSelect.GetComponent<Transform>().Position = new vec3(titleLevelSelectPos.x, titleLevelSelectPosY, titleLevelSelectPos.z);
			TitleQuitGame.GetComponent<Transform>().Position = new vec3(titleQuitGamePos.x, titleQuitGamePosY, titleQuitGamePos.z);
			// Resume Text Bounce if it is close to the position
			if (TitleLevelSelect.GetComponent<TextBounce>().IsPaused() && titleLevelSelectPos.y == displayYPos && Math.Abs(titleLevelSelectPosY - titleLevelSelectPos.y) < titleOffset)
			{
				TitleLevelSelect.GetComponent<TextBounce>().Resume();
			}
			if (TitleQuitGame.GetComponent<TextBounce>().IsPaused() && titleQuitGamePos.y == displayYPos && Math.Abs(titleQuitGamePosY - titleQuitGamePos.y) < titleOffset)
			{
				TitleQuitGame.GetComponent<TextBounce>().Resume();
			}
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

			// Determine what tunnels were approved by both moles
			if (!(selectedOption || selectedLevel || selectedQuit || selectedReturn || selectedTutorial || selectedLevel1 || selectedLevel2) && currentTimer <= 0)
			{
				if (ToOptionSelect.GetComponent<TunnelLogic>().MolesApproved() && !selectedOption)
				{
					selectedOption = true;
					JumpIntoHole();
					ToOptionSelect.GetComponent<TunnelLogic>().ResetMoles();
				}
				else if (ToLevelSelect.GetComponent<TunnelLogic>().MolesApproved() && !selectedLevel)
				{
					selectedLevel = true;
					JumpIntoHole();
					ToLevelSelect.GetComponent<TunnelLogic>().ResetMoles();
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

			// Determine action of selection
			if (currentTimer <= 0 && (selectedOption || selectedLevel || selectedQuit || selectedReturn || selectedTutorial || selectedLevel1 || selectedLevel2))
			{
				currentTimer = 0;

                PhysicsSystem.SetLinearVelocity(Moley.ID, vec3.Zero);
                PhysicsSystem.SetLinearVelocity(Holey.ID, vec3.Zero);

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
					Game.CloseGame();
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

					Scene.TransitionScene("Tutorial", 5f);
				}
                else if (selectedLevel1)
				{
					selectedLevel1 = false;

					Scene.TransitionScene("Level_1", 5f);
				}
                else if (selectedLevel2)
				{
					selectedLevel2 = false;

					Scene.TransitionScene("Level_1", 5f);
				}
			}

			// Return controls to moles if they land on the ground
			if (!(selectedOption || selectedLevel || selectedQuit || selectedReturn || selectedTutorial || selectedLevel1 || selectedLevel2))
			{
				if (!Moley.GetComponent<MoleyController>().isControllable && Moley.GetComponent<MoleyController>().isGrounded && !Moley.GetComponent<CapsuleCollider>().IsTrigger)
				{
					Moley.GetComponent<MoleyController>().isControllable = true;
				}
				if (!Holey.GetComponent<HoleyController>().isControllable && Holey.GetComponent<HoleyController>().isGrounded && !Holey.GetComponent<CapsuleCollider>().IsTrigger)
				{
					Holey.GetComponent<HoleyController>().isControllable = true;
				}
			}
		}

		private void JumpIntoHole()
		{
			if (currentTimer > 0) return;

			Moley.GetComponent<MoleyController>().isControllable = false;
			Holey.GetComponent<HoleyController>().isControllable = false;

			PhysicsSystem.GetLinearVelocity(Moley.ID, out vec3 MoleyVel);
			PhysicsSystem.GetLinearVelocity(Holey.ID, out vec3 HoleyVel);

            Moley.GetComponent<MoleyController>().jumpCancelled = true;
            Holey.GetComponent<HoleyController>().jumpCancelled = true;

            PhysicsSystem.SetLinearVelocity(Moley.ID, vec3.Zero);
            PhysicsSystem.SetLinearVelocity(Holey.ID, vec3.Zero);

            if (MoleyVel.y < 0 || !Moley.GetComponent<MoleyController>().isJumping)
			{
				PhysicsSystem.SetLinearVelocity(Moley.ID, new vec3(0, 40, 0));
			}
			if (HoleyVel.y < 0 || !Holey.GetComponent<HoleyController>().isJumping)
			{
				PhysicsSystem.SetLinearVelocity(Holey.ID, new vec3(0, 40, 0));
			}

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

			PhysicsSystem.SetLinearVelocity(Moley.ID, vec3.Zero);
			PhysicsSystem.SetLinearVelocity(Holey.ID, vec3.Zero);

			PhysicsSystem.SetLinearVelocity(Moley.ID, new vec3(0, 70, 0));
			PhysicsSystem.SetLinearVelocity(Holey.ID, new vec3(0, 70, 0));
		}

		private void DetermineStarsDisplay()
		{
			String holeSceneName = "";
			if (ToTutorialSelect.GetComponent<TunnelLogic>().MolesInside())
			{
                holeSceneName = "Tutorial";
            }
			else if (ToLevel1Select.GetComponent<TunnelLogic>().MolesInside())
            {
                holeSceneName = "Level_1";
            }
            else if (ToLevel2Select.GetComponent<TunnelLogic>().MolesInside())
            {
                holeSceneName = "Level_2";
			}

            Stars1.SetActive(false);
            Stars2.SetActive(false);
            Stars3.SetActive(false);

            int mapStars = 0;
			if (Int32.TryParse(PersistentSystem.GetValue(holeSceneName + "StarsObtained"), out mapStars))
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