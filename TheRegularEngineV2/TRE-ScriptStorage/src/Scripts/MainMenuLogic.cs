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

        // Timer for animation
        float currentTimer = 0;
        float delayJumpingHole = 2f;        

        public void Start()
		{
			UIControls = ECSManager.FindEntityByName("Controls_UI");
            Moley = ECSManager.FindEntityByName("Moley");
            Holey = ECSManager.FindEntityByName("Holey");

            ToOptionSelect = ECSManager.FindEntityByName("ToOptionSelect");
            ToLevelSelect = ECSManager.FindEntityByName("ToLevelSelect");
            ToQuitSelect = ECSManager.FindEntityByName("ToQuitSelect");
            ToReturnSelect = ECSManager.FindEntityByName("ToReturnSelect");
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
            if (ToOptionSelect.GetComponent<TunnelLogic>().MolesApproved())
            {
                selectedOption = true;
                JumpIntoHole();
            }
            if (ToLevelSelect.GetComponent<TunnelLogic>().MolesApproved())
            {
                selectedLevel = true;
                JumpIntoHole();
            }
            if (ToQuitSelect.GetComponent<TunnelLogic>().MolesApproved() && !selectedQuit)
            {
                selectedQuit = true;
                JumpIntoHole();
            }

            if (currentTimer > 0) currentTimer -= Time.deltaTime;

            // Determine action of selection
            if (currentTimer <= 0)
            {
                currentTimer = 0;
                if (selectedQuit)
                {
                    Game.CloseGame();
                }
            }
        }

        private void JumpIntoHole()
        {
            Moley.GetComponent<MoleyController>().isControllable = false;
            Holey.GetComponent<HoleyController>().isControllable = false;
            if (!Moley.GetComponent<MoleyController>().isJumping)
            {
                PhysicsSystem.AddForce(Moley.ID, new vec3(0, 70, 0), ForceMode.VelocityChange);
            }
            if (!Holey.GetComponent<HoleyController>().isJumping)
            {
                PhysicsSystem.AddForce(Holey.ID, new vec3(0, 70, 0), ForceMode.VelocityChange);
            }
            Moley.GetComponent<CapsuleCollider>();
            Holey.GetComponent<CapsuleCollider>();
            currentTimer = delayJumpingHole;
        }
	}
}