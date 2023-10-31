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
        //public Entity Royce;

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
        private float currentTimeBetweenSpawns;
        private bool canSpawnObjs = false;

        private float minRange = 5f;

        private List<Entity> itemsToSpawn = new List<Entity>();

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
            fallingObjPrefabs = new List<Entity> { new Entity(3233608133424215460) };
            maxAmountToSpawn = 2;
            timeBetweenSpawns = 2;
            canSpawnObjs = false;
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
                        itemsToSpawn.Add(ECSManager.Instantiate(fallingObjPrefabs[RandomSpawnObj()], itemToSpawnPos));

                        break;
                    }
                }
            }
        }

        private bool IsPosEmpty(Vector3 position)
        {
            foreach (Entity item in itemsToSpawn)
            {
                if (Vector3.Distance(position, item.transform.Position) < minRange)
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
                itemsToSpawn.Clear();
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
    }

}