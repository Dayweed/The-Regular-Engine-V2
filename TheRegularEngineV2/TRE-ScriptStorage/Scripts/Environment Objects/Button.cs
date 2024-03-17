namespace TRE
{
	public class Button : Entity
	{
		bool isPressed = false;

		public void OnTriggerEnter(System.UInt64 otherID)
		{
			if (EngineGetTag(otherID) == "Red" || EngineGetTag(otherID) == "Blue")
			{
				parenting.GetChildFromName("UpButton").GetComponent<MeshRenderer>().Visible = false;
				isPressed = true;
			}
		}

		public void OnTriggerExit(System.UInt64 otherID)
		{
			if (EngineGetTag(otherID) == "Red" || EngineGetTag(otherID) == "Blue")
			{
				parenting.GetChildFromName("UpButton").GetComponent<MeshRenderer>().Visible = true;
				isPressed = false;
			}
		}

		public bool GetIsButtonPressed()
		{
			return isPressed;
		}
	}
}
