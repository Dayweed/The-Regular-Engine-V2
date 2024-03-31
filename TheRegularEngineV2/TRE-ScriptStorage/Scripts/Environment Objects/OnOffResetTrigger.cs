namespace TRE
{
	public class OnOffResetTrigger : Entity
	{
		OnOffPlatformManager section2Manager;
		OnOffPlatformManager section4Manager;
		OnOffPlatformManager section3Manager;

		public void Start()
		{
			section2Manager = ECSManager.FindEntityByName("OnOff_Platform_Manager_2").GetComponent<OnOffPlatformManager>();
			section4Manager = ECSManager.FindEntityByName("OnOff_Platform_Manager_4").GetComponent<OnOffPlatformManager>();
			section3Manager = ECSManager.FindEntityByName("OnOff_Platform_Manager_3").GetComponent<OnOffPlatformManager>();
		}

		public void OnTriggerStay(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);
			if (other.CompareTag("Red") || other.CompareTag("Blue"))
			{
				section2Manager.MakeAllPlatformsInactive();
				section4Manager.MakeAllPlatformsInactive();
				section3Manager.MakeAllPlatformsInactive();
			}
		}
	}
}
