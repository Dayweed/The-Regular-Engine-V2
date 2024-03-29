using System.Collections.Generic;
using GlmSharp;

namespace TRE
{
	public class RandomizeFallingObjLocation : Entity
	{
		public Entity fallingRockPrefab;
		public Entity CloudEffectPrefab;

		public vec3 size; // Size (in 2D) of the maximum area falling objects can spawn

		// Object spawning
		//public int noOfObjects = 0;
		//public int maxObjects = 8;
		public int maxSearchCount = 4;
		public float minRange = 6.5f;

		// Spawn timing
		public float activeDuration;
		public float minDurationModifier = 0.75f;
		public float maxDurationModifier = 1.3f;
		public float delayDuration = 3;

		private List<Entity> itemsToSpawn = new List<Entity>();
		private List<float> itemsTimer = new List<float>();
		private List<float> itemsDelayTimer = new List<float>();
		private List<vec3> itemsPos = new List<vec3>();
		private List<vec3> itemsDefRot = new List<vec3>();

		public void Start()
		{
			if (Scene.GetSceneName() == "Tutorial")
			{
				if (name == "FallingObj_Spawner_01")
				{
					size = new vec3(30, 0, 30);
					activeDuration = 3;
				}

				if (name == "FallingObj_Spawner_02")
				{
					size = new vec3(30, 0, 30);
					activeDuration = 4;
				}
			}

			itemsToSpawn.Clear();
			itemsTimer.Clear();
			itemsPos.Clear();
			itemsDefRot.Clear();
			itemsDelayTimer.Clear();

			// ID for prefabs are based on resource prefab GUID
			fallingRockPrefab = new Entity(Prefab.GetPrefabIDFromName("FallingRock"));
			CloudEffectPrefab = new Entity(Prefab.GetPrefabIDFromName("CloudEffect"));

			CreateItems();
		}

		// Update is called once per frame
		public void Update()
		{
			UpdateFallingObjs();
		}

		public vec3 SpawnObjPos()
		{
			vec3 spawningPos = this.transform.Position + new vec3(Random.Range(-size.x / 2, size.x / 2), 0, Random.Range(-size.z / 2, size.z / 2));
			return spawningPos;
		}

		public bool AssignNewLocation(out vec3 newpos)
		{
			int searchCount = maxSearchCount;

			//search till limit for place to spawn
			while (searchCount-- > 0)
			{
				//choose random position
				newpos = SpawnObjPos();

				//is this pos empty
				if (IsPosEmpty(newpos))
				{
					return true;
				}
			}

			// Unable to find new pos to spawn
			newpos = vec3.Zero;
			return false;
		}

		private void CreateItems()
		{
			for (int i = parenting.GetTotalChildren() - 1; i >= 0; --i)
			{
				Entity child = parenting.GetChild(i);
				child.parenting.RemoveParent();
				// Only add falling objects with tag
				if (child.GetTag() == "FallingObstacle")
				{
					bool foundSpot = AssignNewLocation(out vec3 pos);
					child.transform.Position = pos;

					itemsToSpawn.Add(child);
					itemsTimer.Add(activeDuration * Random.Range(minDurationModifier, maxDurationModifier));
					itemsPos.Add(pos);
					itemsDefRot.Add(child.transform.Rotation);
					itemsDelayTimer.Add(delayDuration);

					if (!foundSpot) child.SetActive(false);

					//Entity cloud = ECSManager.Instantiate(CloudEffectPrefab);
					//cloud.transform.Position = pos;
				}
			}

			//for (int i = 0; i < itemQuantity; ++i)
			//{
			//	if (noOfObjects < maxObjects)
			//	{
			//		int searchCount = maxObjects * 2;

			//		bool foundSpot = AssignNewLocation(out vec3 pos);
			//		Entity item = ECSManager.Instantiate(fallingRockPrefab);
			//		item.transform.Position = pos;

			//		itemsToSpawn.Add(item);
			//		itemsTimer.Add(activeDuration * Random.Range(minDurationModifier, maxDurationModifier));
			//		itemsPos.Add(pos);
			//		itemsDefRot.Add(item.transform.Rotation);

			//		if (!foundSpot) item.SetActive(false);

			//		++noOfObjects;
			//	}
			//}
		}

		private bool IsPosEmpty(vec3 position)
		{
			for (int i = 0; i < itemsPos.Count; ++i)
			{
				vec3 pos = itemsPos[i];
				vec3 checkPos = new vec3(pos.x, position.y, pos.z);
				if (vec3.Distance(position, checkPos) < minRange && itemsToSpawn[i].GetActive())
				{
					return false;
				}
			}
			return true;
		}

		private void UpdateFallingObjs()
		{
			for (int i = 0; i < itemsTimer.Count; ++i)
			{
				itemsTimer[i] -= Time.deltaTime;

				if (itemsTimer[i] > 0) continue;

				// Deactivate the object and teleport it somewhere else
				itemsToSpawn[i].SetActive(false);
				itemsToSpawn[i].transform.Position = new vec3(0f, 1000f, 0f);
				itemsToSpawn[i].transform.Rotation = itemsDefRot[i];
				itemsToSpawn[i].GetComponent<Rigidbody>().useGravity = false;
				PhysicsSystem.SetLinearVelocity(itemsToSpawn[i].ID, vec3.Zero);

				// Get New Position for object
				bool foundSpot = AssignNewLocation(out vec3 newpos);

				// Ignore if no new spot
				if (!foundSpot) continue;

				itemsDelayTimer[i] -= Time.deltaTime;

				if (itemsDelayTimer[i] > 0) continue;

				// Reset falling object
				itemsPos[i] = newpos;
				itemsToSpawn[i].transform.Position = newpos;
				itemsToSpawn[i].SetActive(true);
				itemsToSpawn[i].GetComponent<Rigidbody>().useGravity = true;
				itemsToSpawn[i].GetComponent<FallingObj>().isGrounded = false;
				PhysicsSystem.SetLinearVelocity(itemsToSpawn[i].ID, vec3.Zero);

				itemsTimer[i] = activeDuration * Random.Range(minDurationModifier, maxDurationModifier);
				itemsDelayTimer[i] = delayDuration;
			}
		}
	}

}