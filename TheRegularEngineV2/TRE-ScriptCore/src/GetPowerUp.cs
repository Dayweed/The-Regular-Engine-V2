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
        public PowerUps powerUpType;

        //private Renderer headPiece;                   // THIS CANT BE DONE YET!
        private Entity playerObj;                       // private Transform playerObj;
        private Entity playerModel;                     // private Transform playerModel;
        private MoleController playerControl;
        private PowerUpManager playerPowerUpManager;

        public GetPowerUp() : base("Mole", ECSManager.FindIDFromName("Mole"))
        {

        }

        private void OnTriggerEnter(/*Collider*/Entity other)
        {
            if (other.CompareTag("Red") || other.CompareTag("Blue"))
            {
                //headPiece = other.GetComponent<Renderer>();                           // THIS CANT BE DONE YET!
                playerModel = other.parenting.GetParent();                              // playerModel = other.transform.parent;
                playerObj = playerModel.parenting.GetParent();                          // playerObj = playerModel.parent;
                //playerControl = playerObj.gameObject.GetComponent<MoleController>();  // THIS CANT BE DONE YET!
                //playerPowerUpManager = playerObj.GetComponent<PowerUpManager>();      // THIS CANT BE DONE YET!

                //if player already has 2 power-ups, don't pick up a 3rd one
                if (playerPowerUpManager.powerUps.Count == 2) return;


                SetToPlayer(other);
            }
        }

        private void SetToPlayer(/*Collider*/Entity other)
        {
            //move the power up gameobj to the player's position
            parenting.SetParent(playerObj);                                     // this.transform.parent = playerObj;
            transform.SetPosition(new Vector3(0, transform.position.y, 0));     // this.transform.localPosition = new Vector3(0, this.transform.localPosition.y, 0);
            playerPowerUpManager.powerUps.Add(this);                            // playerPowerUpManager.powerUps.Add(this.gameObject);
            //this.gameObject.GetComponent<Rigidbody>().isKinematic = true;     // THIS CANT BE DONE YET!
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



    public enum PowerUps { Blueberry, Strawberry }
}