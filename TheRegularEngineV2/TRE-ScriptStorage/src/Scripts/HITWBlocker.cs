using GlmSharp;

namespace TRE
{
	class HITWBlocker : Entity
	{
		private HoleCheckDisplay first;
		private HoleCheckDisplay second;

		private bool doOnce = false;
		private float shrinkSpeed = 25f;

		public void Start()
		{
			first = ECSManager.FindEntityByName("TriggerDisplay_1").GetComponent<HoleCheckDisplay>();
			second = ECSManager.FindEntityByName("TriggerDisplay_2").GetComponent<HoleCheckDisplay>();
		}

		public void Update()
		{
			if (doOnce == false && first.isCompleted && second.isCompleted)
			{
				GetComponent<Transform>().Scale -= new vec3(0f, shrinkSpeed, 0f) * Time.deltaTime;
				GetComponent<BoxCollider>().IsActive = false;

				if (GetComponent<Transform>().Scale.y <= 0)
				{
					GetComponent<Transform>().Scale = new vec3(0, 0, 0);
					doOnce = true;
				}
			}
		}
	}
}
