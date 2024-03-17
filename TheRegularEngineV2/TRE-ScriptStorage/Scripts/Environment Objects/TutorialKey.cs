using GlmSharp;

namespace TRE
{
	public class TutorialKey : Entity
	{
		private Entity FinalPlatform;
		public Entity UpButton;

		public void OnCreate()
		{
			FinalPlatform = ECSManager.FindEntityByName("Final_Platform");
			UpButton = ECSManager.FindEntityByName("UpButton");
		}

		private void OnTriggerEnter(System.UInt64 otherID)
		{
			if (EngineGetTag(otherID) == "Red" || EngineGetTag(otherID) == "Blue")
			{
				TransformSystem.SetPosition(FinalPlatform.ID, new vec3(98.867f, 11.572f, -480.259f));
				UpButton.SetActive(false);

				if (ECSManager.IsValidEntity(5918200844399750609))
					AudioSystem.Play(5918200844399750609);
			}
		}
	}
}
