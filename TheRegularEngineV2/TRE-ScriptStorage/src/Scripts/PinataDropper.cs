using GlmSharp;

namespace TRE
{
	using PS = PhysicsSystem;

	public class PinataDropper : Entity
	{
		private Entity MidPos;

		private Entity Pinata1;
		//private Entity Pinata2;

		private Transform Player1Transform;
		private Transform Player2Transform;

		private vec3 thisPos;

		public float expectedYPos;

		private float lerpSpeed = 0.01f;

		private Entity SpawnRegion;
		private bool spawnRegion;

		public void Start()
		{
			MidPos = ECSManager.FindEntityByName("MidPos");
			SpawnRegion = ECSManager.FindEntityByName("SpawnTrigger");

			Pinata1 = ECSManager.FindEntityByName("Pinata_4");
			//Pinata2 = ECSManager.FindEntityByName("Pinata_5");

			Pinata1.SetActive(false);
		}

		public void Update()
		{
			spawnRegion = IsInsideTrigger(SpawnRegion);

			if (spawnRegion)
			{
				Pinata1.SetActive(true);
			}
		}

		private bool IsInsideTrigger(Entity entity)
		{
			return ECSManager.IsValidEntity(entity.ID) && (PS.IsTriggerEnter(MidPos.ID, entity.ID) || PS.IsTriggerStay(MidPos.ID, entity.ID));
		}
	}
}
