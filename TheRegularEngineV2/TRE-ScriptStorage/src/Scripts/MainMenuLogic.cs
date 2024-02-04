using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using GlmSharp;

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
        float delayJumpingHole = 0.7f;        

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

            // Determine what tunnels were approved by both moles
            if (!(selectedOption || selectedLevel || selectedQuit || selectedReturn || selectedTutorial || selectedLevel1 || selectedLevel2) && currentTimer <= 0)
            {
                if (ToOptionSelect.GetComponent<TunnelLogic>().MolesApproved() && !selectedOption)
                {
                    selectedOption = true;
                    JumpIntoHole();
                    ToOptionSelect.GetComponent<TunnelLogic>().ResetMoles();
                }
                if (ToLevelSelect.GetComponent<TunnelLogic>().MolesApproved() && !selectedLevel)
                {
                    selectedLevel = true;
                    JumpIntoHole();
                    ToLevelSelect.GetComponent<TunnelLogic>().ResetMoles();
                }
                if (ToQuitSelect.GetComponent<TunnelLogic>().MolesApproved() && !selectedQuit)
                {
                    selectedQuit = true;
                    JumpIntoHole();
                    ToQuitSelect.GetComponent<TunnelLogic>().ResetMoles();
                }
                if (ToReturnSelect.GetComponent<TunnelLogic>().MolesApproved() && !selectedReturn)
                {
                    selectedReturn = true;
                    JumpIntoHole();
                    ToReturnSelect.GetComponent<TunnelLogic>().ResetMoles();
                }
                if (ToTutorialSelect.GetComponent<TunnelLogic>().MolesApproved() && !selectedTutorial)
                {
                    selectedTutorial = true;
                    JumpIntoHole();
                    ToTutorialSelect.GetComponent<TunnelLogic>().ResetMoles();
                }
                if (ToLevel1Select.GetComponent<TunnelLogic>().MolesApproved() && !selectedLevel1)
                {
                    selectedLevel1 = true;
                    JumpIntoHole();
                    ToLevel1Select.GetComponent<TunnelLogic>().ResetMoles();
                }
                if (ToLevel2Select.GetComponent<TunnelLogic>().MolesApproved() && !selectedLevel2)
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

                // Force Holey and Moley to stop dropping to do stuff
                Moley.GetComponent<Rigidbody>().useGravity = false;
                Holey.GetComponent<Rigidbody>().useGravity = false;

                if (selectedOption)
                {
                    selectedOption = false;
                }
                if (selectedLevel)
                {
                    selectedLevel = false;

                    // Teleport Moley and Holey to another location
                    vec3 teleportPos = ToReturnSelect.GetComponent<Transform>().Position;
                    Moley.GetComponent<Transform>().Position = new vec3(teleportPos.x - 5, teleportPos.y + 15, teleportPos.z);
                    Holey.GetComponent<Transform>().Position = new vec3(teleportPos.x + 5, teleportPos.y + 15, teleportPos.z);

                    JumpOutHole();
                }
                if (selectedQuit)
                {
                    selectedQuit = false;
                    Game.CloseGame();
                }
                if (selectedReturn)
                {
                    selectedReturn = false;

                    // Teleport Moley and Holey to another location
                    vec3 teleportPos = ToLevelSelect.GetComponent<Transform>().Position;
                    Moley.GetComponent<Transform>().Position = new vec3(teleportPos.x - 5, teleportPos.y + 25, teleportPos.z);
                    Holey.GetComponent<Transform>().Position = new vec3(teleportPos.x + 5, teleportPos.y + 25, teleportPos.z);

                    JumpOutHole();
                }
                if (selectedTutorial)
                {
                    selectedTutorial = false;

                    Scene.TransitionScene("Tutorial", 7f);
                }
                if (selectedLevel1)
                {
                    selectedLevel1 = false;

                    Scene.TransitionScene("Level_1", 7f);
                }
                if (selectedLevel2)
                {
                    selectedLevel2 = false;

                    Scene.TransitionScene("Level_1", 7f);
                }
            }
        }

        private void JumpIntoHole()
        {
            Moley.GetComponent<MoleyController>().isControllable = false;
            Holey.GetComponent<HoleyController>().isControllable = false;

            PhysicsSystem.GetLinearVelocity(Moley.ID, out vec3 MoleyVel);
            PhysicsSystem.GetLinearVelocity(Holey.ID, out vec3 HoleyVel);

            if (!Moley.GetComponent<MoleyController>().isJumping)
            {
                PhysicsSystem.SetLinearVelocity(Moley.ID, new vec3(0, 70, 0));
            }
            if (!Holey.GetComponent<HoleyController>().isJumping)
            {
                PhysicsSystem.SetLinearVelocity(Holey.ID, new vec3(0, 70, 0));
            }

            Moley.GetComponent<CapsuleCollider>().IsTrigger = true;
            Holey.GetComponent<CapsuleCollider>().IsTrigger = true;
            currentTimer = delayJumpingHole;
        }

        private void JumpOutHole()
        {
            // Give controls back to Moley and Holey
            Moley.GetComponent<Rigidbody>().useGravity = true;
            Holey.GetComponent<Rigidbody>().useGravity = true;

            Moley.GetComponent<MoleyController>().isControllable = true;
            Holey.GetComponent<HoleyController>().isControllable = true;

            Moley.GetComponent<CapsuleCollider>().IsTrigger = false;
            Holey.GetComponent<CapsuleCollider>().IsTrigger = false;

            PhysicsSystem.SetLinearVelocity(Moley.ID, vec3.Zero);
            PhysicsSystem.SetLinearVelocity(Holey.ID, vec3.Zero);

            PhysicsSystem.SetLinearVelocity(Moley.ID, new vec3(0, 70, 0));
            PhysicsSystem.SetLinearVelocity(Holey.ID, new vec3(0, 70, 0));
        }
	}
}