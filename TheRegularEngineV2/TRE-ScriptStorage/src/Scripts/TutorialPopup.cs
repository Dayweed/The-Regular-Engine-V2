namespace TRE
{
	public class TutorialPopup : Entity
	{
		private Entity UIPopup1;
		private Entity PopupCollider;
		private bool IsActivated_1 = false;
		private bool HasBeenTriggeredBefore_1 = false;

		private Entity UIPopupBeforeHITW;
		private Entity PopupCollier_BeforeHITW;
		private bool IsActivated_BeforeHITW = false;
		private bool HasBeenTriggeredBefore_BeforeHITW = false;

		private Entity RightHUDRef;
		private Entity LeftHUDRef;
		private Entity TitleStarsHUD;

		private TutorialCameraManager mainCam;

		public void Start()
		{
			UIPopup1 = ECSManager.FindEntityByName("PopupUI1");
			PopupCollider = ECSManager.FindEntityByName("PopUpCollider");
			HasBeenTriggeredBefore_1 = false;
			IsActivated_1 = false;

			RightHUDRef = ECSManager.FindEntityByName("RightCharacter_HUD");
			LeftHUDRef = ECSManager.FindEntityByName("LeftCharacter_HUD");
			TitleStarsHUD = ECSManager.FindEntityByName("TitleStarsCollected");

            UIPopupBeforeHITW = ECSManager.FindEntityByName("PopupUIBeforeHITW");
			PopupCollier_BeforeHITW = ECSManager.FindEntityByName("PopupCollider2");
			IsActivated_BeforeHITW = false;
			HasBeenTriggeredBefore_BeforeHITW = false;

			mainCam = ECSManager.FindEntityByName("CameraManager").GetComponent<TutorialCameraManager>();
		}

		public void Update()
		{
			if (mainCam.preTransitions.preTransitioned)
			{
                RightHUDRef.GetComponent<SpriteRenderer>().isVisible = true;
                LeftHUDRef.GetComponent<SpriteRenderer>().isVisible = true;
                TitleStarsHUD.GetComponent<SpriteRenderer>().isVisible = true;
            }

            if (IsActivated_1 && !IsActivated_BeforeHITW)
			{
				UIPopup1.GetComponent<SpriteRenderer>().isVisible = true;
			}

			if (IsActivated_BeforeHITW)
			{
				UIPopupBeforeHITW.GetComponent<SpriteRenderer>().isVisible = true;
			}

			if (InputSystem.GetKeyHold(InputKeys.Space))
			{
				if (IsActivated_1)
				{
					UIPopup1.GetComponent<SpriteRenderer>().isVisible = false;
					IsActivated_1 = !IsActivated_1;
				}
				if (IsActivated_BeforeHITW)
				{
					UIPopupBeforeHITW.GetComponent<SpriteRenderer>().isVisible = false;
					IsActivated_BeforeHITW = !IsActivated_BeforeHITW;
				}
			}
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
	}
}
