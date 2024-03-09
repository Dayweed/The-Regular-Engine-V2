namespace TRE
{
	class Button : Entity
	{
		bool isPressed = false;

		void OnTriggerEnter(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);

			if (EngineGetTag(otherID) == "Red" || EngineGetTag(otherID) == "Blue")
			{
				parenting.GetChildFromName("UpButton").GetComponent<MeshRenderer>().Visible = false;
				isPressed = true;
			}
		}

		void OnTriggerExit(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);

			if (EngineGetTag(otherID) == "Red" || EngineGetTag(otherID) == "Blue")
			{
				parenting.GetChildFromName("UpButton").GetComponent<MeshRenderer>().Visible = true;
				isPressed = false;
			}
		}

		public bool GetIsPressed()
		{
			return isPressed;
		}
	}
}
