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
        public PowerUpUI MyPowerUpUI;
        public List<Entity> powerUps = new List<Entity>();

        //private Entity baseForm;
        //private Entity blueberryForm;
        //private Entity strawberryForm;

        private bool usePower = false;

        public PowerUpManager()
        {

        }

        private void OnCreate()
        {
            //baseForm = this.parenting.GetChild(0);
            //blueberryForm = this.parenting.GetChild(1);
            //strawberryForm = this.parenting.GetChild(2);
        }

        public void SwapPowerUps() //if holding 2 powerups, swap between them. if currently using a power-up, will swap into the other form
        {
            if (powerUps.Count < 2) return;

            powerUps.Reverse();

            //Entity tempContainer = powerUps[0];
            //powerUps.RemoveAt(0);
            //powerUps.Add(tempContainer);

            //if (usePower)
            //{
            //    ActivatePowerUp(powerUps[0].GetComponent<GetPowerUp>()); // activating the main power up        // THIS CANT BE DONE YET
            //}
        }

        public void DropMain() //pop off the main power up
        {
            if (powerUps.Count == 0) return;

            GetPowerUp mainPowerUp = powerUps[0].GetComponent<GetPowerUp>();
            mainPowerUp.ReleasePowerUp();
            powerUps.RemoveAt(0);

            MyPowerUpUI.UpdateUI(powerUps);
        }

        public void LoseMain()
        {
            if (powerUps.Count == 0) return;

            ECSManager.DestroyEntity(powerUps[0].ID);
            powerUps.RemoveAt(0);

            if (MyPowerUpUI != null)
            {
                MyPowerUpUI.UpdateUI(powerUps);
            }
        }
    }
}
