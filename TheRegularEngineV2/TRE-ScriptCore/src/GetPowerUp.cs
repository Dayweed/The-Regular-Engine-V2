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

        //private Renderer headPiece;                   // THIS CANT BE DONE YET!
        private Entity playerObj;                       // private Transform playerObj;
        private Entity playerModel;                     // private Transform playerModel;
        private MoleController playerControl;
        private PowerUpManager playerPowerUpManager;

        public GetPowerUp()
        {

        }

        private void OnTriggerStay(/*Collider*/System.UInt64 otherID)
        {
            Entity other = new Entity(otherID);
            Debug.Log("Triggered with " + ECSManager.FindNameFromID(other.ID));
        }

        private void OnCollisionStay(System.UInt64 otherID)
        {
            Entity other = new Entity(otherID);
            Debug.Log("Collided with " + ECSManager.FindNameFromID(other.ID));

            if (other.CompareTag("Red") || other.CompareTag("Blue"))
            {
                //headPiece = other.GetComponent<Renderer>();                           // THIS CANT BE DONE YET!
                //playerModel = other.parenting.GetParent();                              // playerModel = other.transform.parent;
                //playerObj = playerModel.parenting.GetParent();                          // playerObj = playerModel.parent;
                playerObj = other;

                playerControl = playerObj.GetComponent<MoleController>();               //playerControl = playerObj.gameObject.GetComponent<MoleController>();  // THIS CANT BE DONE YET!
                
                if (!ECSManager.IsValidEntity(playerObj.parenting.GetChild(0).ID))
                {
                    Debug.LogError("Could not find player child (" + playerObj.parenting.GetChild(0).ID + ")");
                    return;
                }
                playerPowerUpManager = playerObj.parenting.GetChild(0).GetComponent<PowerUpManager>();        //playerPowerUpManager = playerObj.GetComponent<PowerUpManager>();      // THIS CANT BE DONE YET!

                if (playerControl == null)
                {
                    Debug.LogError("Could not find playerControl");
                    return;
                }
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

                SetToPlayer(other);
            }
        }

        private void SetToPlayer(Entity other)
        {
            //move the power up gameobj to the player's position
            parenting.SetParent(playerObj);                                     // this.transform.parent = playerObj;

            this.transform.Position =  new Vector3(0, this.transform.Position.y, 0);     // this.transform.localPosition = new Vector3(0, this.transform.localPosition.y, 0);

            playerPowerUpManager.powerUps.Add(this);                            // playerPowerUpManager.powerUps.Add(this.gameObject);

            RigidBodySystem.SetKinematic(ID, true);                             //this.gameObject.GetComponent<Rigidbody>().isKinematic = true;     // THIS CANT BE DONE YET!
            //this.gameObject.GetComponent<Collider>().enabled = false;         // THIS CANT BE DONE YET!
            //this.gameObject.GetComponent<RotateObj>().enabled = false;        // THIS CANT BE DONE YET!
            parenting.GetChild(1).SetActive(false);                             //this.transform.GetChild(1).gameObject.SetActive(false);
        }

        public void TurnOnVisuals()
        {
            parenting.RemoveParent();                                           //this.transform.parent = null;
            parenting.GetChild(1).SetActive(true);                              //this.transform.GetChild(1).gameObject.SetActive(true);

            TurnOnCollider();//Invoke(nameof(TurnOnCollider), 0.5f);            // THIS CANT BE DONE YET!
        }

        public void TurnOnCollider()
        {
            //this.gameObject.GetComponent<Collider>().enabled = true;          // THIS CANT BE DONE YET!
            //this.gameObject.GetComponent<RotateObj>().enabled = true;         // THIS CANT BE DONE YET!
        }
    }



    public enum PowerUpsType { Blueberry, Strawberry }
}