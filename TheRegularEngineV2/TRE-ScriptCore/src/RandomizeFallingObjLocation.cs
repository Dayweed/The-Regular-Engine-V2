using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;

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

        public Vector3 size;

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
        private List<Vector3> itemsPos = new List<Vector3>();

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
            maxAmountToSpawn = 3;
            maxObjects = 3;
            timeBetweenSpawns = 2;
            size = new Vector3(30, 0, 50);
            canSpawnObjs = true;
            dropDuration = 5.0f;
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

        public Vector3 SpawnObjPos()
        {
            Vector3 spawningPos = this.transform.Position + new Vector3(Random.Range(-size.x / 2, size.x / 2),
                                                                            Random.Range(-size.y / 2, size.y / 2),
                                                                                Random.Range(-size.z / 2, size.z / 2));
            

            return spawningPos;
        }

        public int RandomSpawnObj()
        {
            int spawnObj = Random.Range(0, fallingObjPrefabs.Count);
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
                        Vector3 itemToSpawnPos = SpawnObjPos();

                        //is this pos empty
                        if (IsPosEmpty(itemToSpawnPos))
                        {
                            //yes, so add to list
                            itemsToSpawn.Add(ECSManager.Instantiate(fallingObjPrefabs[RandomSpawnObj()], itemToSpawnPos, default, Vector3.one));
                            itemsTimer.Add(dropDuration);
                            itemsPos.Add(itemToSpawnPos);
                            break;
                        }
                    }
                    ++noOfObjects;
                }
            }
        }

        private bool IsPosEmpty(Vector3 position)
        {
            //foreach (Entity item in itemsToSpawn)
            //{
            //    Debug.Log("E " + Vector3.Distance(position, item.transform.Position));
            //    if (Vector3.Distance(position, item.transform.Position) < minRange)
            //    {
            //        return false;
            //    }
            //}
            foreach (Vector3 pos in itemsPos)
            {
                Vector3 checkPos = new Vector3(pos.x, position.y, pos.z);
                if (Vector3.Distance(position, checkPos) < minRange)
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
                        Vector3 itemToSpawnPos = SpawnObjPos();

                        if (IsPosEmpty(itemToSpawnPos))
                        {
                            //choose random position
                            Vector3 itemPos = SpawnObjPos();

                            //is this pos empty
                            if (IsPosEmpty(itemPos))
                            {
                                TransformSystem.SetPosition(itemsToSpawn[i].ID, itemPos);
                                TransformSystem.SetRotation(itemsToSpawn[i].ID, Vector3.zero);
                                itemsPos[i] = itemPos;

                                itemsTimer[i] = dropDuration;
                            }
                        }
                    }
                }
            }
        }
    }

}