using GlmSharp;

namespace TRE
{
	public class VFX_ParticleDisplay : Entity
	{
		Entity mainCamera = null;

		float displayDuration = 1f;
		float displayTimer = 0f;
		bool displaying = false;

		public void Start()
		{
			displayTimer = displayDuration;

			mainCamera = ECSManager.FindEntityByName("Main Camera");

			//Display();
		}

		public void Update()
		{
			// Always look at the main camera
			if (mainCamera != null)
			{
				// Rotate Character to look at target
				vec2 rotAxis = MathF.GetLookAtAxis(transform.Position, mainCamera.transform.Position);
				transform.Rotation = new vec3(rotAxis.x, rotAxis.y, 0);
			}

			if (!displaying) return;

			displayTimer -= Time.deltaTime;
			if (displayTimer > 0f) return;

			displayTimer = displayDuration;
			displaying = false;

			SpriteSystem.SetSprite3DVisibility(ID, false);
			//DestroySelf();
		}

		public void Display()
		{
			displaying = true;
			displayTimer = displayDuration;
			SpriteSystem.SetSprite3DVisibility(ID, true);
		}
	}
}