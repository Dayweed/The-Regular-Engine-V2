using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;

namespace TRE
{
    public class GetPowerUp : Entity
    {
        public PowerUpsType powerUpType;
        public Entity PowerUpManagerObj;

        public string mole1tag = "Red";
        public string mole2tag = "Player";

        private bool collected;
        private int collectedIndex;

        private float cooldownDuration;
        private float cooldownCurrent;

        //private Renderer headPiece;                   // THIS CANT BE DONE YET!
        private Entity playerObj;                       // private Transform playerObj;
        private Entity playerModel;                     // private Transform playerModel;

        private PowerUpManager playerPowerUpManager;

        public GetPowerUp()
        {

        }

        public void OnCreate()
        {
            collected = false;
            cooldownDuration = 5f;
            cooldownCurrent = 0f;
        }

        private void OnTriggerStay(/*Collider*/System.UInt64 otherID)
        {
            Entity other = new Entity(otherID);
            //Debug.Log("Triggered with " + ECSManager.FindNameFromID(other.ID));

            if (other.CompareTag("Ground") && HasComponent<Rigidbody>())
            {
                //RemoveComponent<Rigidbody>();
                cooldownCurrent = 0;
                return;
            }

            if (collected || cooldownCurrent > 0) return;

            if (other.CompareTag(mole1tag) || other.CompareTag(mole2tag))
            {
                //headPiece = other.GetComponent<Renderer>();                           // THIS CANT BE DONE YET!
                //playerModel = other.parenting.GetParent();                              // playerModel = other.transform.parent;
                //playerObj = playerModel.parenting.GetParent();                          // playerObj = playerModel.parent;
                
                playerObj = other;
                PowerUpManagerObj = playerObj.parenting.GetChildFromName("Power Manager");

                if (!ECSManager.IsValidEntity(PowerUpManagerObj.ID))
                {
                    Debug.LogError("Could not find PowerUpManagerObj (" + PowerUpManagerObj.ID + ")");
                    return;
                }
                playerPowerUpManager = PowerUpManagerObj.GetComponent<PowerUpManager>();        //playerPowerUpManager = playerObj.GetComponent<PowerUpManager>();

                if (playerPowerUpManager == null)
                {
                    Debug.LogError("Could not find playerPowerUpManager");
                    return;
                }
                if (playerPowerUpManager.powerUps == null)
                {
                    Debug.LogError("Could not find playerPowerUpManager.powerUps");
                    return;
                }

                //if player already has 2 power-ups, don't pick up a 3rd one
                if (playerPowerUpManager.powerUps.Count == 2) return;

                Debug.Log("Collided with " + ECSManager.FindNameFromID(other.ID));
                collectedIndex = playerPowerUpManager.powerUps.Count;
                playerPowerUpManager.powerUps.Add(this);                            // playerPowerUpManager.powerUps.Add(this.gameObject);

                SetToPlayer();

                // Is Mole 1
                if (playerObj.CompareTag(mole1tag))
                {
                    MoleController controller = playerObj.GetComponent<MoleController>();               //playerControl = playerObj.gameObject.GetComponent<MoleController>();

                    if (controller == null)
                    {
                        Debug.LogError("Could not find playerControl");
                        return;
                    }

                    // Check what type of powerup it is (Default Blueberry for now)
                    if (CompareTag("Strawberry"))
                    {
                        controller.haveStrawberry = true;
                    }
                    else if (CompareTag("Blueberry"))
                    {
                        controller.haveBlueberry = true;
                    }

                    collected = true;
                }
                // Is Mole 2
                else if (playerObj.CompareTag(mole2tag))
                {
                    MoleController2 controller = playerObj.GetComponent<MoleController2>();               //playerControl = playerObj.gameObject.GetComponent<MoleController>();

                    if (controller == null)
                    {
                        Debug.LogError("Could not find playerControl");
                        return;
                    }

                    // Check what type of powerup it is (Default Blueberry for now)
                    if (CompareTag("Strawberry"))
                    {
                        controller.haveStrawberry = true;
                    }
                    else if (CompareTag("Blueberry"))
                    {
                        controller.haveBlueberry = true;
                    }

                    collected = true;
                }

                ReleasePowerUp();
            }
        }

        private void OnCollisionStay(System.UInt64 otherID)
        {
            Entity other = new Entity(otherID);
            //Debug.Log("Collided with " + ECSManager.FindNameFromID(other.ID));
        }

        public void Update()
        {
            cooldownCurrent -= Time.deltaTime;
            SetToPlayer();
        }

        private void SetToPlayer()
        {
            //move the power up gameobj to the player's position
            //parenting.SetParent(playerObj);                                     // this.transform.parent = playerObj;

            //this.transform.Position =  new Vector3(0, this.transform.Position.y, 0);     // this.transform.localPosition = new Vector3(0, this.transform.localPosition.y, 0);

            if (playerObj == null || ECSManager.IsValidEntity(playerObj.ID) == false) return;

            Vector3 newPos = playerObj.transform.Position;
            newPos.y += playerObj.transform.Scale.y * 5 * (collectedIndex + 1);
            transform.Position = newPos;
            //transform.Position = newPos;

            //RigidBodySystem.SetKinematic(ID, false);                             //this.gameObject.GetComponent<Rigidbody>().isKinematic = true;     // THIS CANT BE DONE YET!
            //this.gameObject.GetComponent<Collider>().enabled = false;         // THIS CANT BE DONE YET!
            //this.gameObject.GetComponent<RotateObj>().enabled = false;        // THIS CANT BE DONE YET!
            //SetActive(false);                                                   //this.transform.GetChild(1).gameObject.SetActive(false);
        }

        public void ReleasePowerUp()
        {
            playerObj = null;
            collected = false;
            AddComponent<Rigidbody>();
            PhysicsSystem.AddForce(this.ID, new Vector3(0, 35, 0), ForceMode.VelocityChange);
            cooldownCurrent = cooldownDuration;
        }

        public void TurnOnVisuals()
        {
            // No clue what this does yet
            //parenting.RemoveParent();                                           //this.transform.parent = null;
            //parenting.GetChild(1).SetActive(true);                              //this.transform.GetChild(1).gameObject.SetActive(true);

            //TurnOnCollider();//Invoke(nameof(TurnOnCollider), 0.5f);            // THIS CANT BE DONE YET!
        }

        public void TurnOnCollider()
        {
            //this.gameObject.GetComponent<Collider>().enabled = true;          // THIS CANT BE DONE YET!
            //this.gameObject.GetComponent<RotateObj>().enabled = true;         // THIS CANT BE DONE YET!
        }
    }



    public enum PowerUpsType { Blueberry, Strawberry }
}