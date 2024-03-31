namespace TRE
{
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

		public void Start()
		{
			TriggerPlatformCollider = parenting.GetChildFromName("TriggerPlatform");
			Holey = ECSManager.FindEntityByName("Holey");
			Moley = ECSManager.FindEntityByName("Moley");
			directionChangeSFX_1 = ECSManager.FindIDFromName("SFX_Platform_1");
			directionChangeSFX_2 = ECSManager.FindIDFromName("SFX_Platform_2");
			directionChangeSFX_3 = ECSManager.FindIDFromName("SFX_Platform_3");
			directionChangeSFX_4 = ECSManager.FindIDFromName("SFX_Platform_4");
		}

		public void Update()
		{
			// Make Moley or Holey follow the platform if within trigger box
			if (HasComponent<DirectPathfinding>() && ECSManager.IsValidEntity(TriggerPlatformCollider.ID))
			{
				// Check if is in trigger
				if (PhysicsSystem.IsTriggerStay(TriggerPlatformCollider.ID, Holey.ID))
				{
					Holey.GetComponent<Transform>().Position += transform.Position - GetComponent<DirectPathfinding>().oldPosition;
				}
				if (PhysicsSystem.IsTriggerStay(TriggerPlatformCollider.ID, Moley.ID))
				{
					Moley.GetComponent<Transform>().Position += transform.Position - GetComponent<DirectPathfinding>().oldPosition;
				}

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
	}
}