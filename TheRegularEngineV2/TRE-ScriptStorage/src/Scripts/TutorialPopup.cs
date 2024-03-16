using System.Linq.Expressions;

namespace TRE
{
	public class TutorialPopup : Entity
	{
		private Entity UIPopup1;
        private Entity UIPopUp1_controls;
		private Entity PopupCollider;
		private bool IsActivated_1 = false;
		private bool HasBeenTriggeredBefore_1 = false;

		private Entity UIPopupBeforeHITW;
		private Entity UIPopupBeforeHITW_Controls;
		private Entity PopupCollier_BeforeHITW;
		private bool IsActivated_BeforeHITW = false;
		private bool HasBeenTriggeredBefore_BeforeHITW = false;

		private Entity RightHUDRef;
		private Entity LeftHUDRef;
		private Entity TitleStarsHUD;

        private bool controller1 = false;
        private bool lastController1 = false;
		private bool changeUI = false;
		

		//Audio
		private ulong uiSFX;
		private bool uiSFXPlayed = false;
		private bool playSFX = false;


		private TutorialCameraManager mainCam;

		public void Start()
		{
			UIPopup1 = ECSManager.FindEntityByName("PopupUI1");
			UIPopUp1_controls = ECSManager.FindEntityByName("PopupUI1_controls");
			PopupCollider = ECSManager.FindEntityByName("PopUpCollider");
			HasBeenTriggeredBefore_1 = false;
			IsActivated_1 = false;

			RightHUDRef = ECSManager.FindEntityByName("RightCharacter_HUD");
			LeftHUDRef = ECSManager.FindEntityByName("LeftCharacter_HUD");
			TitleStarsHUD = ECSManager.FindEntityByName("TitleStarsCollected");

            UIPopupBeforeHITW = ECSManager.FindEntityByName("PopupUIBeforeHITW");
			UIPopupBeforeHITW_Controls = ECSManager.FindEntityByName("PopupUIBeforeHITW_controls");
			PopupCollier_BeforeHITW = ECSManager.FindEntityByName("PopupCollider2");
			IsActivated_BeforeHITW = false;
			HasBeenTriggeredBefore_BeforeHITW = false;

			mainCam = ECSManager.FindEntityByName("CameraManager").GetComponent<TutorialCameraManager>();

			uiSFX = ECSManager.FindIDFromName("SFX_UI");
		}

		public void Update()
		{
			//Controller Check for UI
            controller1 = InputSystem.GetControllerConnected(0);

			playSFX = false;

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

            if (changeUI && controller1)
            {
                UIPopUp1_controls.GetComponent<SpriteRenderer>().Texture = "ui-button-a.png";
				UIPopupBeforeHITW_Controls.GetComponent<SpriteRenderer>().Texture = "ui-button-a.png";
            }
			else if (changeUI && !controller1)
            {
				UIPopUp1_controls.GetComponent<SpriteRenderer>().Texture = "ui-button-space.png";
				UIPopupBeforeHITW_Controls.GetComponent<SpriteRenderer>().Texture = "ui-button-space.png";
            }

			if (mainCam.preTransitions.preTransitioned)
			{
                RightHUDRef.GetComponent<SpriteRenderer>().isVisible = true;
                LeftHUDRef.GetComponent<SpriteRenderer>().isVisible = true;
                TitleStarsHUD.GetComponent<SpriteRenderer>().isVisible = true;

			}

            if (IsActivated_1 && !IsActivated_BeforeHITW)
			{
				UIPopup1.GetComponent<SpriteRenderer>().isVisible = true;
				UIPopUp1_controls.GetComponent<SpriteRenderer>().isVisible = true;

				playSFX = true;
			}

			if (IsActivated_BeforeHITW)
			{
				UIPopupBeforeHITW.GetComponent<SpriteRenderer>().isVisible = true;
				UIPopupBeforeHITW_Controls.GetComponent<SpriteRenderer>().isVisible = true;

				playSFX = true;

			}

			if (InputSystem.GetKeyHold(InputKeys.Space) || InputSystem.GetControllerButtonPress(0, InputSystem.Button.A) || InputSystem.GetControllerButtonPress(1, InputSystem.Button.A))
			{
				if (IsActivated_1)
				{
					UIPopup1.GetComponent<SpriteRenderer>().isVisible = false;
					UIPopUp1_controls.GetComponent<SpriteRenderer>().isVisible = false;
					IsActivated_1 = !IsActivated_1;
				}
				if (IsActivated_BeforeHITW)
				{
					UIPopupBeforeHITW.GetComponent<SpriteRenderer>().isVisible = false;
					UIPopupBeforeHITW_Controls.GetComponent<SpriteRenderer>().isVisible = false;
					IsActivated_BeforeHITW = !IsActivated_BeforeHITW;
				}

				playSFX = false;

			}

			HandleSFX();	
		}

		public void OnTriggerStay(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);

			// Check is interacted with moles players
			if ((EngineGetTag(otherID) == "Red" || EngineGetTag(otherID) == "Blue"))
			{
				if (!HasBeenTriggeredBefore_1 && this.ID.Equals(PopupCollider.ID))
				{
					IsActivated_1 = true;
					HasBeenTriggeredBefore_1 = true;
					IsActivated_BeforeHITW = false;
					HasBeenTriggeredBefore_BeforeHITW = false;
				}
				if (!HasBeenTriggeredBefore_BeforeHITW && this.ID.Equals(PopupCollier_BeforeHITW.ID))
				{
					IsActivated_BeforeHITW = true;
					HasBeenTriggeredBefore_BeforeHITW = true;
				}
			}
		}

		private void HandleSFX()
		{
			if (!uiSFXPlayed && playSFX)
			{
				if (ECSManager.IsValidEntity(uiSFX))
				{
					AudioSystem.Play(uiSFX);
					uiSFXPlayed = true;
					playSFX = false;
				}
			}
			else if (uiSFXPlayed && !playSFX)
			{
				if (ECSManager.IsValidEntity(uiSFX))
				{
					AudioSystem.Stop(uiSFX);
					uiSFXPlayed = false;
				}
			}

			playSFX = false;
		}
	}
}
