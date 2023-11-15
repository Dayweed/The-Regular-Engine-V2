using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using GlmSharp;

namespace TRE
{
    public class HumanCentipede : Entity
    {
        public int blueBridge;
        public Entity Royce;

        // How to call base class constructor to access Entity-like Properties
        public HumanCentipede()
        {

        }

        public void OnEnable()
        {
            Debug.Log("OnEnable");
        }

        public void OnDisable()
        {
            Debug.Log("OnDisable");
        }

        public void OnDestroy()
        {
            Debug.Log("OnDestroy");
        }

        public void Start()
        {
            Debug.Log("Testing on Finding Scripts");
            Debug.Log("Found HumanCentipede in GameObject (3): " + Script.HaveScript(ECSManager.FindIDFromName("GameObject (3)"), "TRE.RandomizeFallingObjLocation"));
            Debug.Log("Testing Getting Scripts");
            RandomizeFallingObjLocation test = Script.GetScript<RandomizeFallingObjLocation>(ECSManager.FindIDFromName("GameObject (3)"), "TRE.RandomizeFallingObjLocation");
            Debug.Log("Successfully get w/o errors, maxAmountToSpawn is " + test.maxAmountToSpawn + ", changing to 10");
            test.maxAmountToSpawn = 10;
            Debug.Log("Changed value, maxAmountToSpawn is " + test.maxAmountToSpawn);
            Debug.Log("Checking new value by getting again to see if it is still " + Script.GetScript<RandomizeFallingObjLocation>(ECSManager.FindIDFromName("GameObject (3)"), "TRE.RandomizeFallingObjLocation").maxAmountToSpawn + ".");

            Debug.Log("Testing GetComponent... ");
            Entity ent = ECSManager.FindEntityByName("GameObject (3)");
            RandomizeFallingObjLocation entget = ent.GetComponent<RandomizeFallingObjLocation>();
            Debug.Log("entget maxAmountToSpawn is " + entget.maxAmountToSpawn);

            Debug.Log("Testing Invalid GetComponent... ");
            HumanCentipede failedget = ent.GetComponent<HumanCentipede>();
            Debug.Log("Invalid GetComponent HumanCentipede: " + (failedget == null));

            Debug.Log("My Value is " + blueBridge);
        }

        public void Update()
        {
            // Console.WriteLine("this " + this.transform.position.x + ", " + this.transform.position.y + ", " + this.transform.position.z);
        }

        public void LateUpdate()
        {
            // Console.WriteLine("this " + this.transform.position.x + ", " + this.transform.position.y + ", " + this.transform.position.z);
        }
    }

    public class RandomizeFallingObjLocation : Entity
    {
        public List<Entity> fallingObjPrefabs;
        public List<Entity> fallingObjRNG;

        public vec3 size;

        public int maxAmountToSpawn;
        public int timeBetweenSpawns;
        public int noOfObjects;
        public int maxObjects;
        public float dropDuration;

        private float currentTimeBetweenSpawns;
        private bool canSpawnObjs = false;

        private float minRange;

        private List<Entity> itemsToSpawn = new List<Entity>();
        private List<float> itemsTimer = new List<float>();
        private List<vec3> itemsPos = new List<vec3>();
        private List<vec3> itemsDefRot = new List<vec3>();

        public RandomizeFallingObjLocation()
        {

        }

        public void OnCreate()
        {

        }

        public void Start()
        {
            //canSpawnObjs = true;

            // ID for prefabs are based on resource prefab GUID
            fallingObjPrefabs = new List<Entity> { new Entity(11822093139939255162), new Entity(8829880216004354162) };
            fallingObjRNG = new List<Entity>(fallingObjPrefabs);
            maxAmountToSpawn = 3;
            maxObjects = 3;
            timeBetweenSpawns = 2;
            size = new vec3(20, 0, 50);
            canSpawnObjs = true;
            dropDuration = 3.5f;
            minRange = 5.5f;
        }

        // Update is called once per frame
        public void Update()
        {
            if (ID == 0) return;

            if (InputSystem.GetKeyDown(InputKeys.T))
            {
                canSpawnObjs = !canSpawnObjs;
            }

            StartTimer();
            UpdateItems();
        }

        public vec3 SpawnObjPos()
        {
            vec3 spawningPos = this.transform.Position + new vec3(Random.Range(-size.x / 2, size.x / 2),
                                                                            Random.Range(-size.y / 2, size.y / 2),
                                                                                Random.Range(-size.z / 2, size.z / 2));
            return spawningPos;
        }

        public int RandomSpawnObj()
        {
            if (fallingObjRNG.Count == 0)
            {
                fallingObjRNG = new List<Entity>(fallingObjPrefabs);
            }
            int spawnObj = Random.Range(0, fallingObjRNG.Count);
            fallingObjRNG.RemoveAt(spawnObj);
            return spawnObj;
        }

        private void CreateItems(int itemQuantity)
        {
            for (int i = 0; i < itemQuantity; ++i)
            {
                if (noOfObjects < maxObjects)
                {
                    int searchCount = maxAmountToSpawn * 2;

                    //search till limit for place to spawn
                    while (searchCount-- > 0)
                    {
                        //choose random position
                        vec3 itemToSpawnPos = SpawnObjPos();

                        //is this pos empty
                        if (IsPosEmpty(itemToSpawnPos))
                        {
                            //yes, so add to list
                            Entity item = ECSManager.Instantiate(fallingObjPrefabs[RandomSpawnObj()]);
                            item.transform.Position = itemToSpawnPos;
                            itemsToSpawn.Add(item);
                            itemsTimer.Add(dropDuration);
                            itemsPos.Add(itemToSpawnPos);
                            itemsDefRot.Add(item.transform.Rotation);
                            break;
                        }
                    }
                    ++noOfObjects;
                }
            }
        }

        private bool IsPosEmpty(vec3 position)
        {
            //foreach (Entity item in itemsToSpawn)
            //{
            //    Debug.Log("E " + Vector3.Distance(position, item.transform.Position));
            //    if (Vector3.Distance(position, item.transform.Position) < minRange)
            //    {
            //        return false;
            //    }
            //}
            foreach (vec3 pos in itemsPos)
            {
                vec3 checkPos = new vec3(pos.x, position.y, pos.z);
                if (vec3.Distance(position, checkPos) < minRange)
                {
                    return false;
                }
            }
            return true;
        }

        private void OnDrawGizmos()
        {
            // TO DO DRAW OUTLINE OF WHATEVER DATA (transform.position)
            //Gizmos.color = Color.cyan;
            //Gizmos.DrawWireCube(this.transform.position, size);
        }

        public void StartTimer()
        {
            if (!canSpawnObjs) return;

            if (currentTimeBetweenSpawns > 0)
            {
                //itemsToSpawn.Clear();
                currentTimeBetweenSpawns -= Time.deltaTime;
            }
            else
            {
                for (int i = 0; i < maxAmountToSpawn; i++)
                {
                    CreateItems(maxAmountToSpawn);
                }

                currentTimeBetweenSpawns = timeBetweenSpawns;
            }
        }

        public void UpdateItems()
        {
            for (int i = 0; i < itemsTimer.Count; ++i)
            {
                itemsTimer[i] -= Time.deltaTime;
                if (itemsTimer[i] < 0)
                {

                    //is this pos empty
                    int searchCount = maxAmountToSpawn * 5;

                    while (searchCount-- > 0)
                    {
                        //choose random position
                        vec3 itemToSpawnPos = SpawnObjPos();

                        if (IsPosEmpty(itemToSpawnPos))
                        {
                            //choose random position
                            vec3 itemPos = SpawnObjPos();

                            //is this pos empty
                            if (IsPosEmpty(itemPos))
                            {
                                PhysicsSystem.SetLinearVelocity(itemsToSpawn[i].ID, vec3.Zero);
                                TransformSystem.SetPosition(itemsToSpawn[i].ID, itemPos);
                                TransformSystem.SetRotation(itemsToSpawn[i].ID, itemsDefRot[i]);
                                itemsPos[i] = itemPos;

                                itemsTimer[i] = dropDuration;

                                //AudioSystem.Play(16157905661456646599);
                            }
                        }
                    }
                }
            }
        }
    }

}