namespace TRE
{
	public class Button : Entity
	{
		bool isPressed = false;

		#region Audio
		private ulong platformSFX;
		#endregion

		public void Start()
		{
			platformSFX = ECSManager.FindIDFromName("SFX_ActivatePlatform");
		}

		public void OnTriggerEnter(System.UInt64 otherID)
		{
			if (EngineGetTag(otherID) == "Red" || EngineGetTag(otherID) == "Blue")
			{
				parenting.GetChildFromName("UpButton").GetComponent<MeshRenderer>().Visible = false;
				isPressed = true;

				if (ECSManager.IsValidEntity(platformSFX))
					AudioSystem.Play(platformSFX);
			}
		}

		public void OnTriggerExit(System.UInt64 otherID)
		{
			if (EngineGetTag(otherID) == "Red" || EngineGetTag(otherID) == "Blue")
			{
				parenting.GetChildFromName("UpButton").GetComponent<MeshRenderer>().Visible = true;
				isPressed = false;

				if (ECSManager.IsValidEntity(platformSFX))
					AudioSystem.Play(platformSFX);
			}
		}

		public bool GetIsButtonPressed()
		{
			return isPressed;
		}
	}
}
