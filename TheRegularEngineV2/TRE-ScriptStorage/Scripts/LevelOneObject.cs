using GlmSharp;

namespace TRE
{
	public class LevelOneObject : Entity
	{
		// private Entity LevelObject;
		private Entity LvlObjUI;

		public bool pickedUp = false;

		public void OnCreate()
		{
			// LevelObject = ECSManager.FindEntityByName("LevelObject_PickMe");
			LvlObjUI = ECSManager.FindEntityByName("LevelObject");
		}

		public void Update()
		{
			if (pickedUp)
				LvlObjUI.GetComponent<SpriteRenderer>().isVisible = true;
		}

		public void OnTriggerEnter(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);

			if (other.CompareTag("Red") || other.CompareTag("Blue"))
			{
				GetComponent<MeshRenderer>().Visible = false;
				pickedUp = true;
                LvlObjUI.GetComponent<VFX_Emerge>().Emerge(vec3.Zero, new vec3(-840f, -280f, 0f), new vec3(1.5f, 1.5f, 1));
            }
		}
	}
}
