using System;
using System.Reflection;
using System.Runtime.CompilerServices;

namespace TRE
{
	using PS = PhysicsSystem;

	public class Main
	{
		// Scripting Initialization
		HumanCentipede humanCentipedo = new HumanCentipede();
		RandomizeFallingObjLocation testingRandoFall = new RandomizeFallingObjLocation();

        //create the test object and temp object not too sure if the temp object is linked in some 
        public Entity Temp = new Entity("Temp");
        public Entity Test = new Entity("Test");
        public Entity Plane_collider = new Entity("Plane collider");

        //check if player is on the ground (for now , just a plane)
        private bool isGrounded = true;
        //Maxium height the player can jump
        private Vector3 maxHeight = new Vector3(0, 10000, 0);


        //check if player used super power
        private bool isScaled = false;
        private float defaultScale = 1;
        private float superScale = 50;

        private bool hasInitalized;

        public Main()
        {
            //Temp.id = ECSManager.CreateEntity(Temp.name);
            //Console.WriteLine("Hello World from C#!");

            Test.id = ECSManager.FindIDFromName(Test.name);
            Plane_collider.id = ECSManager.FindIDFromName(Plane_collider.name);
            hasInitalized = false;
		}


        // is this even running??
        public void Start()
        {
            //humanCentipedo.Start();
            //testingRandoFall.Start();
            hasInitalized = false;
            Console.WriteLine("HELLO FROM SCRIPT-START\n");
            // this is not running...
		}

        public void Update()
        {
            if (!hasInitalized) Initialize();
            // Script calling
            //humanCentipedo.Update();
            //testingRandoFall.Update();

            // Move The Test Object 
            TransformSystem.GetPosition(Test.id, out Vector3 pos);

            // if the player JUST starts to touch the ground OR has been chilling on the ground for a while
            isGrounded = PS.IsCollisionEnter(Test.id, Plane_collider.id) || PS.IsCollisionStay(Test.id, Plane_collider.id);

            Vector3 dirVec = new Vector3(0, 0, 0);

            if (InputSystem.GetKeyDown(InputKeys.W))
            {
                dirVec.z += 1;
                //PS.AddForce(Mole.id, dirVec);
            }

            if (InputSystem.GetKeyDown(InputKeys.S))
            {
                dirVec.z += -1;
                //PS.AddForce(Mole.id, dirVec);
            }

            if (InputSystem.GetKeyDown(InputKeys.A))
            {
                dirVec.x += -1;
            }

            if (InputSystem.GetKeyDown(InputKeys.D))
            {
                dirVec.x += 1;
            }

            if (InputSystem.GetKeyDown(InputKeys.Space))
            {
                if (isGrounded)
                    Jump(maxHeight); // uh oh beeeg number
            }

            if (InputSystem.GetKeyDown(InputKeys.E))
            {
                if (!isScaled)
                {
                    PS.ResizeSphereCollider(Test.id, superScale);
                    isScaled = true;
                }
                else if (isScaled)
                {
                    PS.ResizeSphereCollider(Test.id, defaultScale);
                    isScaled = false;
                }
            }

            dirVec.Normalize();

            // if you're on the ground and not moving
            if (dirVec.Magnitude() == 0 && isGrounded)
            {
                // stop the entity from moving
                PS.SetLinearVelocity(Test.id, Vector3.zero);
            }
            else
            {
                Vector3 tmp = dirVec * 60;
                PS.AddForce(Test.id, tmp*2, PS.ForceMode.VelocityChange);
            }
        }

        private void Jump(Vector3 JumpHeight)
        {
            PS.AddForce(Test.id, JumpHeight, PS.ForceMode.Force);
            // PS.AddForce(Test.id, 35, PS.ForceMode.VelocityChange);
        }

        private void Initialize()
        {
            PS.ConstrainRotationX(Test.id, true);
            // PS.ConstrainRotationY(Test.id, true);
            PS.ConstrainRotationZ(Test.id, true);
            hasInitalized = true;
        }
    }
}
