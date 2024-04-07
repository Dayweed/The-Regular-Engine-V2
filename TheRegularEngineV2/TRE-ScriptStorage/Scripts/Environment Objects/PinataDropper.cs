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
		private Entity Pinata8;
		private Entity Pinata9;
		private Entity Pinata10;
		private Entity Pinata11;
		private Entity Pinata12;
		private Entity Pinata13;
		private Entity Pinata14;

		// private Transform Player1Transform;
		// private Transform Player2Transform;

		// private vec3 thisPos;

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
			Pinata8 = ECSManager.FindEntityByName("Pinata_15");
			Pinata9 = ECSManager.FindEntityByName("Pinata_16");
			Pinata10 = ECSManager.FindEntityByName("Pinata_17");
			Pinata11 = ECSManager.FindEntityByName("Pinata_18");
			Pinata12 = ECSManager.FindEntityByName("Pinata_19");
			Pinata13 = ECSManager.FindEntityByName("Pinata_20");
			Pinata14 = ECSManager.FindEntityByName("Pinata_21");

			Pinata1.SetActive(false);
			Pinata1.GetComponent<CapsuleCollider>().IsActive = false;
			Pinata2.SetActive(false);
			Pinata2.GetComponent<CapsuleCollider>().IsActive = false;
			Pinata3.SetActive(false);
			Pinata3.GetComponent<CapsuleCollider>().IsActive = false;
			Pinata4.SetActive(false);
			Pinata4.GetComponent<CapsuleCollider>().IsActive = false;
			Pinata5.SetActive(false);
			Pinata5.GetComponent<CapsuleCollider>().IsActive = false;
			Pinata6.SetActive(false);
			Pinata6.GetComponent<CapsuleCollider>().IsActive = false;
			Pinata7.SetActive(false);
			Pinata7.GetComponent<CapsuleCollider>().IsActive = false;
			Pinata8.SetActive(false);
			Pinata8.GetComponent<CapsuleCollider>().IsActive = false;
			Pinata9.SetActive(false);
			Pinata9.GetComponent<CapsuleCollider>().IsActive = false;
			Pinata10.SetActive(false);
			Pinata10.GetComponent<CapsuleCollider>().IsActive = false;
			Pinata11.SetActive(false);
			Pinata11.GetComponent<CapsuleCollider>().IsActive = false;
			Pinata12.SetActive(false);
			Pinata12.GetComponent<CapsuleCollider>().IsActive = false;
			Pinata13.SetActive(false);
			Pinata13.GetComponent<CapsuleCollider>().IsActive = false;
			Pinata14.SetActive(false);
			Pinata14.GetComponent<CapsuleCollider>().IsActive = false;
		}

		public void Update()
		{
			spawnRegion = IsInsideTrigger(SpawnRegion);

			if (spawnRegion)
			{
				Pinata1.SetActive(true);
				Pinata1.GetComponent<CapsuleCollider>().IsActive = true;
				Pinata2.SetActive(true);
				Pinata2.GetComponent<CapsuleCollider>().IsActive = true;
				Pinata3.SetActive(true);
				Pinata3.GetComponent<CapsuleCollider>().IsActive = true;
				Pinata4.SetActive(true);
				Pinata4.GetComponent<CapsuleCollider>().IsActive = true;
				Pinata5.SetActive(true);
				Pinata5.GetComponent<CapsuleCollider>().IsActive = true;
				Pinata6.SetActive(true);
				Pinata6.GetComponent<CapsuleCollider>().IsActive = true;
				Pinata7.SetActive(true);
				Pinata7.GetComponent<CapsuleCollider>().IsActive = true;
				Pinata8.SetActive(true);
				Pinata8.GetComponent<CapsuleCollider>().IsActive = true;
				Pinata9.SetActive(true);
				Pinata9.GetComponent<CapsuleCollider>().IsActive = true;
				Pinata10.SetActive(true);
				Pinata10.GetComponent<CapsuleCollider>().IsActive = true;
				Pinata11.SetActive(true);
				Pinata11.GetComponent<CapsuleCollider>().IsActive = true;
				Pinata12.SetActive(true);
				Pinata12.GetComponent<CapsuleCollider>().IsActive = true;
				Pinata13.SetActive(true);
				Pinata13.GetComponent<CapsuleCollider>().IsActive = true;
				Pinata14.SetActive(true);
				Pinata14.GetComponent<CapsuleCollider>().IsActive = true;
			}

		}

		private bool IsInsideTrigger(Entity entity)
		{
			return ECSManager.IsValidEntity(entity.ID) && (PS.IsTriggerEnter(MidPos.ID, entity.ID) || PS.IsTriggerStay(MidPos.ID, entity.ID));
		}
	}
}
