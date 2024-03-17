namespace TRE
{
	public class HoleCheckTrigger : Entity
	{
		public HoleCheckDisplay triggerDisplay;   // Updated by parent if it is HoleCheckDisplay
		public bool isHit = false;

		// Tag should be either [TriggerMoleyStrawberry], [TriggerMoleyBlueberry], [TriggerHoleyStrawberry], [TriggerHoleyBlueberry]

		public void OnTriggerStay(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);

			// Check is interacted with moles players
			if ((EngineCompareTag(ID, "TriggerMoleyStrawberry") && EngineGetTag(otherID) == "Red" && other.GetComponent<MoleyController>().isScaled && other.GetComponent<MoleyController>().mainStrawberry)
			 || (EngineCompareTag(ID, "TriggerMoleyBlueberry") && EngineGetTag(otherID) == "Red" && other.GetComponent<MoleyController>().isScaled && other.GetComponent<MoleyController>().mainBlueberry)
			 || (EngineCompareTag(ID, "TriggerHoleyStrawberry") && EngineGetTag(otherID) == "Blue" && other.GetComponent<HoleyController>().isScaled && other.GetComponent<HoleyController>().mainStrawberry)
			 || (EngineCompareTag(ID, "TriggerHoleyBlueberry") && EngineGetTag(otherID) == "Blue" && other.GetComponent<HoleyController>().isScaled && other.GetComponent<HoleyController>().mainBlueberry))
			{
				isHit = true;
				triggerDisplay.CheckTrigger();
			}
		}

		public void OnTriggerExit(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);

			// Check is interacted with moles players
			if ((EngineCompareTag(ID, "TriggerMoleyStrawberry") && EngineGetTag(otherID) == "Red" && other.GetComponent<MoleyController>().isScaled && other.GetComponent<MoleyController>().mainStrawberry)
			 || (EngineCompareTag(ID, "TriggerMoleyBlueberry") && EngineGetTag(otherID) == "Red" && other.GetComponent<MoleyController>().isScaled && other.GetComponent<MoleyController>().mainBlueberry)
			 || (EngineCompareTag(ID, "TriggerHoleyStrawberry") && EngineGetTag(otherID) == "Blue" && other.GetComponent<HoleyController>().isScaled && other.GetComponent<HoleyController>().mainStrawberry)
			 || (EngineCompareTag(ID, "TriggerHoleyBlueberry") && EngineGetTag(otherID) == "Blue" && other.GetComponent<HoleyController>().isScaled && other.GetComponent<HoleyController>().mainBlueberry))
			{
				isHit = false;
				triggerDisplay.CheckTrigger();
			}
		}
	}
}