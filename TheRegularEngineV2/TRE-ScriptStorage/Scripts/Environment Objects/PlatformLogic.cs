namespace TRE
{
	using GlmSharp;
	using AS = AudioSystem;

	public class PlatformLogic : Entity
	{
		private Entity TriggerPlatformCollider;
		private Entity Holey;
		private Entity Moley;
		private ulong directionChangeSFX_1;
		private ulong directionChangeSFX_2;
		private ulong directionChangeSFX_3;
		private ulong directionChangeSFX_4;

        const float trembleDuration = 0.5f;		// The total amount of time in seconds for the platform to tremble before moving.
        const float trembleAmplitude = 0.2f;	// The amount in units to tremble by.
        const float trembleFrequency = 30.0f;   // The speed of the trembling.
		float globalTimer = 0f;

        public void Start()
		{
			TriggerPlatformCollider = parenting.GetChildFromName("TriggerPlatform");
			Holey = ECSManager.FindEntityByName("Holey");
			Moley = ECSManager.FindEntityByName("Moley");
			directionChangeSFX_1 = ECSManager.FindIDFromName("SFX_Platform_1");
			directionChangeSFX_2 = ECSManager.FindIDFromName("SFX_Platform_2");
			directionChangeSFX_3 = ECSManager.FindIDFromName("SFX_Platform_3");
			directionChangeSFX_4 = ECSManager.FindIDFromName("SFX_Platform_4");
            globalTimer = 0f;
        }

		public void Update()
		{
			// Make Moley or Holey follow the platform if within trigger box
			if (HasComponent<DirectPathfinding>() && ECSManager.IsValidEntity(TriggerPlatformCollider.ID))
			{
				globalTimer += Time.deltaTime;

                // Check if is in trigger
                if (PhysicsSystem.IsTriggerStay(TriggerPlatformCollider.ID, Holey.ID))
				{
					Holey.GetComponent<Transform>().Position += transform.Position - GetComponent<DirectPathfinding>().oldPosition;
				}
				if (PhysicsSystem.IsTriggerStay(TriggerPlatformCollider.ID, Moley.ID))
				{
					Moley.GetComponent<Transform>().Position += transform.Position - GetComponent<DirectPathfinding>().oldPosition;
				}

				// Shake before moving
				DirectPathfinding DP = GetComponent<DirectPathfinding>();
				/*if (DP.currentDelayTime > 0f && DP.currentDelayTime < trembleDuration)
				{
					Tremble();
                }*/

				// Play Audio SFX
				if (GetComponent<DirectPathfinding>().directionChange)
				{
					if (name == "Platform_1")
					{
						if (ECSManager.IsValidEntity(directionChangeSFX_1))
						{
							AS.Play(directionChangeSFX_1);
						}
					}
					if (name == "Platform_2" || name == "Platform_3" || name == "Platform_4")
					{
						if (ECSManager.IsValidEntity(directionChangeSFX_2))
						{
							AS.Play(directionChangeSFX_2);
						}
					}
					if (name == "Platform_5" || name == "Platform_6")
					{
						if (ECSManager.IsValidEntity(directionChangeSFX_3))
						{
							AS.Play(directionChangeSFX_3);
						}
					}
					if (name == "Platform_7" || name == "Platform_8" || name == "Platform_9")
					{
						if (ECSManager.IsValidEntity(directionChangeSFX_4))
						{
							AS.Play(directionChangeSFX_4);
						}
					}
				}
			}
		}

		public void OnCollisionStay(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);
			if (other.CompareTag("Red") || other.CompareTag("Blue") || other.CompareTag("Strawberry") || other.CompareTag("Blueberry"))
			{
				other.GetComponent<Transform>().Position += (transform.Position - GetComponent<DirectPathfinding>().oldPosition) * Time.deltaTime;
			}
		}

		public void Tremble()
		{
            vec3 pos = transform.Position;
            pos.x += (Random.Range(0, 2) == 1 ? 1 : -1) * trembleAmplitude * MathF.Sin(globalTimer * trembleFrequency);
            pos.z += (Random.Range(0, 2) == 1 ? 1 : -1) * trembleAmplitude * MathF.Sin(globalTimer * trembleFrequency);
			transform.Position = pos;
        }
	}
}