using GlmSharp;
using System.Runtime.Remoting.Metadata.W3cXsd2001;

namespace TRE
{
	public class LevelOneObject : Entity
	{
		// private Entity LevelObject;
		private Entity LvlObjUI;
		public Entity mainCamera;

		public bool pickedUp = false;

		private const float rotationSpeed = 20;
		float rotSpeed = rotationSpeed;

		//VFX
		private Entity mRadialVFX;
		private bool mIsRadialVFX = true;

		private ulong sfx;

		public void OnCreate()
		{
			// LevelObject = ECSManager.FindEntityByName("LevelObject_PickMe");
			mainCamera = ECSManager.FindEntityByName("Main Camera");
			LvlObjUI = ECSManager.FindEntityByName("LevelObject");
			mRadialVFX = ECSManager.FindEntityByName("RadialEffectVFX");

			sfx = ECSManager.FindIDFromName("SFX_CollectLevelObj");
			mIsRadialVFX = true;
		}

		public void Update()
		{
			if (pickedUp)
				LvlObjUI.GetComponent<SpriteRenderer>().isVisible = true;
			else
			{
				// Spin object
				TransformSystem.GetRotation(this.ID, out vec3 rot);

				rot.y += rotSpeed * Time.deltaTime;

				if (rot.y >= 90)
					rotSpeed = rotationSpeed * -1;

				else if (rot.y <= -90)
					rotSpeed = rotationSpeed;

				TransformSystem.SetRotation(this.ID, new vec3(0, rot.y, 0));
			}

			if (pickedUp)
			{
				if (mIsRadialVFX)
				{
					mIsRadialVFX = false;
					SpriteSystem.SetSprite3DVisibility(mRadialVFX.ID, false);
				}
			}
		}

		public void OnTriggerEnter(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);

			if (!pickedUp && (other.CompareTag("Red") || other.CompareTag("Blue")))
			{

				if (ECSManager.IsValidEntity(sfx))
				{
					AudioSystem.Play(sfx);
				}

				GetComponent<MeshRenderer>().Visible = false;
				pickedUp = true;
				LvlObjUI.GetComponent<VFX_Emerge>().Emerge(vec3.Zero, new vec3(-840f, -280f, 0f), new vec3(1.5f, 1.5f, 1));
				PersistentSystem.SetValue(GetTag(), "true");
			}
		}
	}
}
