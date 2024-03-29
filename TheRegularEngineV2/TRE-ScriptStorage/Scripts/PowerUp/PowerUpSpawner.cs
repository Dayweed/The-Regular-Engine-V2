using GlmSharp;

namespace TRE
{
	//private InputActionAsset inputActionAsset; //contains the action maps, actions, bindings and controls schemes
	//private InputActionMap player; //the main action map to control the player

	public class PowerUpSpawner : Entity
	{
		public Entity spawnedPowerUp;
		public Entity powerUpPrefab;
		private vec3 positionOffset;

		public bool spawned;
		public float cooldownCurrent;
		public const float cooldownDuration = 5.0f;

		private int numSpawn = 0;

		public void OnCreate()
		{
			if (CompareTag("SpawnStrawberry"))
				powerUpPrefab = new Entity(Prefab.GetPrefabIDFromName("Strawberry"));
			else if (CompareTag("SpawnBlueberry"))
				powerUpPrefab = new Entity(Prefab.GetPrefabIDFromName("Blueberry"));
			

			positionOffset = new vec3(0, 2, 0);

			cooldownCurrent = 0;

			spawned = false;
		}

		public void Update()
		{
			// Check if power up is collected
			if (spawned && (spawnedPowerUp == null || !ECSManager.IsValidEntity(spawnedPowerUp.ID) || spawnedPowerUp.GetComponent<GetPowerUp>().collected))
			{
				spawned = false;
				cooldownCurrent = cooldownDuration;
			}

			// Spawn Power Up
			if (!spawned)
			{
				cooldownCurrent -= Time.deltaTime;
				if (cooldownCurrent <= 0)
				{
					SpawnPowerUp();
					spawned = true;
				}
			}
		}

		public void SpawnPowerUp()
		{
			spawnedPowerUp = ECSManager.Instantiate(powerUpPrefab);
			spawnedPowerUp.transform.Position = transform.Position + spawnedPowerUp.transform.Scale.y + positionOffset;
			spawnedPowerUp.transform.Rotation = transform.Rotation;
			spawnedPowerUp.GetComponent<Rigidbody>().useGravity = true;
			spawnedPowerUp.Rename("Spawned Powerup " + spawnedPowerUp.name + " " + numSpawn++);
			spawnedPowerUp.parenting.SetParent(this);
			//Debug.Log("spawnedPowerUp have GetPowerUp? " + spawnedPowerUp.HasComponent<GetPowerUp>());
			//Debug.Log("But can I access its values? collected = " + spawnedPowerUp.GetComponent<GetPowerUp>().collected);
			//Debug.Log("Is my name the same? " + spawnedPowerUp.name + " = " + spawnedPowerUp.GetComponent<GetPowerUp>().ReturnName());
			PhysicsSystem.SetLinearVelocity(spawnedPowerUp.ID, vec3.Zero);
			PhysicsSystem.AddForce(spawnedPowerUp.ID, new vec3(0, 50, 0), ForceMode.VelocityChange);
		}
	}
}