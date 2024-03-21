using GlmSharp;

namespace TRE
{
	using PS = PhysicsSystem;

	public class PinataDropper : Entity
	{
		private Entity MidPos;

		private Entity Pinata1;
		private Entity Pinata2;
		private Entity Pinata3;
		private Entity Pinata4;
		private Entity Pinata5;
		private Entity Pinata6;
		private Entity Pinata7;

		// private Transform Player1Transform;
		// private Transform Player2Transform;

		private vec3 thisPos;

		public float expectedYPos;

		private const float lerpSpeed = 0.01f;

		private Entity SpawnRegion;
		private bool spawnRegion;

		public void Start()
		{
			MidPos = ECSManager.FindEntityByName("MidPos");
			SpawnRegion = ECSManager.FindEntityByName("SpawnTrigger");

			Pinata1 = ECSManager.FindEntityByName("Pinata_8");
			Pinata2 = ECSManager.FindEntityByName("Pinata_9");
			Pinata3 = ECSManager.FindEntityByName("Pinata_10");
			Pinata4 = ECSManager.FindEntityByName("Pinata_11");
			Pinata5 = ECSManager.FindEntityByName("Pinata_12");
			Pinata6 = ECSManager.FindEntityByName("Pinata_13");
			Pinata7 = ECSManager.FindEntityByName("Pinata_14");

			Pinata1.SetActive(false);
			Pinata2.SetActive(false);
			Pinata3.SetActive(false);
			Pinata4.SetActive(false);
			Pinata5.SetActive(false);
			Pinata6.SetActive(false);
			Pinata7.SetActive(false);
		}

		public void Update()
		{
			spawnRegion = IsInsideTrigger(SpawnRegion);

			if (spawnRegion)
			{
				Pinata1.SetActive(true);
				Pinata2.SetActive(true);
				Pinata3.SetActive(true);
				Pinata4.SetActive(true);
				Pinata5.SetActive(true);
				Pinata6.SetActive(true);
				Pinata7.SetActive(true);
			}
				
		}

		private bool IsInsideTrigger(Entity entity)
		{
			return ECSManager.IsValidEntity(entity.ID) && (PS.IsTriggerEnter(MidPos.ID, entity.ID) || PS.IsTriggerStay(MidPos.ID, entity.ID));
		}
	}
}
