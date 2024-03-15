namespace TRE
{
	public class OnOffResetTrigger : Entity
	{
		OnOffPlatformManager section2Manager;

		void Start()
		{
			section2Manager = ECSManager.FindEntityByName("OnOff_Platform_Manager_2").GetComponent<OnOffPlatformManager>();
		}

		void OnTriggerStay(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);
			if (other.CompareTag("Red") || other.CompareTag("Blue"))
				section2Manager.MakeAllPlatformsInactive();
		}
	}
}
