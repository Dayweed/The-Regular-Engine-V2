using GlmSharp;

namespace TRE
{
	using TS = TransformSystem;

	public class TextBounce : Entity
	{
		bool pause = false;

		float initialYPos;
		float timer = 0;
		const float height = 10f;
		const float period = 5f;

		int dir = 1;

		public void Start()
		{
			timer = 0;
			TS.GetPosition(this.ID, out vec3 pos);
			initialYPos = pos.y;

			// Custom direction for result logic
			if (Scene.GetSceneName() == "ResultScreen")
			{
				if (name == "BackgroundTitle")
				{
					dir = -1;
				}
			}
		}

		public void Update()
		{
			if (pause) return;

			timer += Time.GetDeltaTime();
			TS.GetPosition(this.ID, out vec3 pos);
			float value = initialYPos + height * MathF.Sin(period * timer) * dir;
			pos.y = value;
			TS.SetPosition(this.ID, pos);
		}

		public void Pause()
		{
			pause = true;
		}

		public void Resume()
		{
			pause = false;

			timer = 0;
			TS.GetPosition(this.ID, out vec3 pos);
			initialYPos = pos.y;

			// Custom direction for result logic
			if (Scene.GetSceneName() == "ResultScreen")
			{
				if (name == "BackgroundTitle")
				{
					dir = -1;
				}
			}
		}

		public bool IsPaused()
		{
			return pause;
		}
	}
}