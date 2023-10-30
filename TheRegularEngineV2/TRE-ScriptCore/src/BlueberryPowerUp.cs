using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;

namespace TRE
{
    public class BlueberryPowerUp : Entity
    {
        //private InputActionAsset inputActionAsset; //contains the action maps, actions, bindings and controls schemes
        //private InputActionMap player; //the main action map to control the player

        private Entity blueberryModel;
        public Vector3 changeToThisSize;
        public float colliderRadius;
        public float colliderHeight;

        private bool changeSize = false;

        private void Start()
        {
            Debug.Log(this.name + "finding blueberry");
        }

        private void OnCreate()
        {
            //inputActionAsset = this.GetComponent<PlayerInput>().actions;
            //inputActionAsset = this.GetComponentInParent<PlayerInput>().actions;
            //player = inputActionAsset.FindActionMap("Player");

            blueberryModel = this.parenting.GetChild(1);                //blueberryModel = this.transform.GetChild(1).gameObject;
        }

        private void OnEnable()
        {
            //player.FindAction("Action").performed += DoSizeChange;    // THIS CANT BE DONE YET
            DoSizeChange();
        }

        private void OnDisable()
        {
            //player.FindAction("Action").performed -= DoSizeChange;    // THIS CANT BE DONE YET
            DoSizeChange();
        }

        private void DoSizeChange(/*InputAction.CallbackContext context*/)
        {
            if (!changeSize)
            {
                Debug.Log("changing size");
                this.transform.Scale=changeToThisSize;  //change the way it looks
                                                              //change the collider size
                                                              //this.transform.parent.GetComponent<CapsuleCollider>().radius = colliderRadius;

                PhysicsSystem.ResizeCapsuleCollider(this.ID, colliderRadius, colliderHeight); //this.parenting.parent.GetComponent<CapsuleCollider>().height = colliderHeight;

                changeSize = true;
            }
            else
            {
                Debug.Log("changing back");
                ChangeToDefault();
            }
        }

        public bool GetSizeChange()
        {
            return changeSize;
        }

        public void ChangeToDefault()
        {
            this.transform.Scale = Vector3.one;

            //change back collider size
            PhysicsSystem.ResizeCapsuleCollider(this.ID, colliderRadius, 2); //this.parenting.GetParent().GetComponent<CapsuleCollider>().height = 2;    // THIS CANT BE DONE YET

            changeSize = false;
        }
    }
}
