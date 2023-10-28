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

        private bool hasInitalized = false;

        public Main()
        {
            //Temp.id = ECSManager.CreateEntity(Temp.name);
            //Console.WriteLine("Hello World from C#!");

            Test.id = ECSManager.FindIDFromName(Test.name);
            Plane_collider.id = ECSManager.FindIDFromName(Plane_collider.name);
        }


        // is this even running??
        public void Start()
        {
            //humanCentipedo.Start();
            //testingRandoFall.Start();
        }

        public void Update()
        {
            if (!hasInitalized) Initialize();
            // Script calling
            //humanCentipedo.Update();
            //testingRandoFall.Update();

            // Move The Test Object 
            TransformSystem.GetPosition(Test.id, out Vector3 pos);
            //

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
                // if the player JUST starts to touch the ground OR has been chilling on the ground for a while
                isGrounded = PS.IsCollisionEnter(Test.id, Plane_collider.id) || PS.IsCollisionStay(Test.id, Plane_collider.id);

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

            Vector3 tmp = dirVec * 60;

            PS.AddForce(Test.id, tmp, PS.ForceMode.Force);
        }

        private void Jump(Vector3 JumpHeight)
        {
            PhysicsSystem.AddForce(Test.id, JumpHeight, PS.ForceMode.Force);
            // PS.AddForce(Test.id, 35, PS.ForceMode.VelocityChange);
        }

        private void Initialize()
        {
            PhysicsSystem.ConstrainRotationX(Test.id, true);
            PhysicsSystem.ConstrainRotationY(Test.id, true);
            //PhysicsSystem.ConstrainRotationZ(Test.id, true);
            hasInitalized = true;
        }
    }
}
