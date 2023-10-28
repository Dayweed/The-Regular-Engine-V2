using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;

namespace TRE
{
    public class PowerUpManager : Entity
    {
        public List<Entity> powerUps = new List<Entity>();

        private Entity baseForm;
        private Entity blueberryForm;
        private Entity strawberryForm;

        private bool usePower = false;

        public PowerUpManager() : base("spawner", ECSManager.FindIDFromName("spawner"))
        {

        }

        private void Awake()
        {
            baseForm = this.parenting.GetChild(0);
            blueberryForm = this.parenting.GetChild(1);
            strawberryForm = this.parenting.GetChild(2);
        }

        public void SwapPowerUps() //if holding 2 powerups, swap between them. if currently using a power-up, will swap into the other form
        {
            if (powerUps.Count < 2) return;

            Entity tempContainer = powerUps[0];
            powerUps.RemoveAt(0);
            powerUps.Add(tempContainer);

            if (usePower)
            {
                //ActivatePowerUp(powerUps[0].GetComponent<GetPowerUp>()); // activating the main power up        // THIS CANT BE DONE YET
            }
        }

        public void DropMain() //pop off the main power up
        {
            /* // THIS CANT BE DONE YET!
            GetPowerUp mainPowerUp = powerUps[0].GetComponent<GetPowerUp>();
            Rigidbody mainRb = powerUps[0].GetComponent<Rigidbody>();
            RemoveMain();
            mainRb.isKinematic = false;
            mainRb.AddForce(Vector3.up * MathF.Sqrt(2 * -2f * Physics.gravity.y) + this.transform.forward * -2f, ForceMode.VelocityChange);
            mainPowerUp.TurnOnVisuals();
            */
        }

        public void ToggleMainPowerUp() //turn on or off the main powerup
        {
            if (powerUps.Count == 0) return;

            //GetPowerUp mainPowerUp = powerUps[0].GetComponent<GetPowerUp>();    // THIS CANT BE DONE YET

            //if power up is currently in use
            if (usePower)
            {
                DeactivatePowerUps();
            }
            //if power up isn't being used
            else
            {
                //ActivatePowerUp(mainPowerUp);     // THIS CANT BE DONE YET
            }
        }

        private void ActivatePowerUp(GetPowerUp thisPower) //change to the model that represents the powerup
        {
            baseForm.SetActive(false);

            switch (thisPower.powerUpType)
            {
                case PowerUps.Blueberry:
                    blueberryForm.SetActive(true);
                    strawberryForm.SetActive(false);
                    break;
                case PowerUps.Strawberry:
                    blueberryForm.SetActive(false);
                    strawberryForm.SetActive(true);
                    break;
                default:
                    break;
            }

            usePower = true;
        }

        private void DeactivatePowerUps() //turn back to base model
        {
            baseForm.SetActive(true);
            blueberryForm.SetActive(false);
            strawberryForm.SetActive(false);

            usePower = false;
        }

        public void RemoveMain() //remove the first powerup
        {
            DeactivatePowerUps();
            powerUps.RemoveAt(0);
        }

    }
}
