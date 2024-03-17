namespace TRE
{
	public class LevelOneObjectHitW : Entity
	{
		private Entity LevelObject;

		private LevelOneObject LevelOneObj;

		public void OnCreate()
		{
			LevelOneObj = ECSManager.FindEntityByName("LevelObject_PickMe").GetComponent<LevelOneObject>();
			LevelObject = ECSManager.FindEntityByName("LevelObject_Picked");
			LevelObject.GetComponent<MeshRenderer>().Visible = false;
		}

		private void OnTriggerEnter(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);

			if (other.CompareTag("Red") || other.CompareTag("Blue"))
			{
				if (LevelOneObj.pickedUp)
				{
					LevelObject.GetComponent<MeshRenderer>().Visible = true;
				}
				this.SetActive(false);
			}
		}
	}
}
