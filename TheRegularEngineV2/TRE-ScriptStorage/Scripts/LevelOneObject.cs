using GlmSharp;

namespace TRE
{
	public class LevelOneObject : Entity
	{
		// private Entity LevelObject;
		private Entity LvlObjUI;

		public bool pickedUp = false;

		private const float rotationSpeed = 20;
		float rotSpeed = rotationSpeed;

		//VFX
		private Entity mRadialVFX;
		private bool mIsRadialVFX = false;
		private float mRadialVFXDuration = 3f;

		public void OnCreate()
		{
			// LevelObject = ECSManager.FindEntityByName("LevelObject_PickMe");
			LvlObjUI = ECSManager.FindEntityByName("LevelObject");
            mRadialVFX = ECSManager.FindEntityByName("RadialEffectVFX");
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

			if (mIsRadialVFX)
			{
				mRadialVFXDuration -= Time.GetDeltaTime();
				if (mRadialVFXDuration < 0 )
				{
					mRadialVFXDuration = 3f;
					mIsRadialVFX = false;
					SpriteSystem.SetSprite3DVisibility(mRadialVFX.ID, false);
				}
			}
		}

		private void SetRadialEffect(Entity mole)
		{
			vec3 MolePos = mole.GetComponent<Transform>().Position;
			MolePos.y += 5;
			mRadialVFX.GetComponent<Transform>().Position = MolePos;
            SpriteSystem.SetSprite3DVisibility(mRadialVFX.ID, true);
            mIsRadialVFX = true;
        }

		public void OnTriggerEnter(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);

			if (other.CompareTag("Red") || other.CompareTag("Blue"))
			{
				GetComponent<MeshRenderer>().Visible = false;
				pickedUp = true;
				LvlObjUI.GetComponent<VFX_Emerge>().Emerge(vec3.Zero, new vec3(-840f, -280f, 0f), new vec3(1.5f, 1.5f, 1));
				SetRadialEffect(other);
			}
		}
	}
}
