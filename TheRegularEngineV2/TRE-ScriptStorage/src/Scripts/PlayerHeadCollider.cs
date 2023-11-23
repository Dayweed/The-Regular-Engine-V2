using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TRE
{
    class PlayerHeadCollider : Entity
    {
        private Entity playerObj;
        private string mole1tag = "RedCollider";
        private string mole2tag = "BlueCollider";
        bool isPlayer1 = false;
        bool isPlayer2 = false;
        private vec3 offset;

        private void SetToPlayer()
        {
            PhysicsSystem.GetColliderOffset(playerObj.ID, out offset);

            if (playerObj == null || ECSManager.IsValidEntity(playerObj.ID) == false)
            {
                return;
            }
            if(this.CompareTag(mole1tag))
            {
                vec3 newPos = playerObj.transform.Position;
                newPos.y += playerObj.transform.Scale.y * 5f;
                transform.Position = newPos;
            }
            else if(this.CompareTag(mole2tag))
            {
                vec3 newPos = playerObj.transform.Position;
                newPos.y += playerObj.GetComponent<HoleyController>().currentHeight + playerObj.GetComponent<HoleyController>().currentRadius + offset.y + 1f;


                if (playerObj.GetComponent<HoleyController>().mainBlueberry == true && playerObj.GetComponent<HoleyController>().isScaled == true)
                {
                    newPos.y = playerObj.GetComponent<HoleyController>().currentHeight + playerObj.GetComponent<HoleyController>().currentRadius + offset.y + 10f;
                }
                transform.Position = newPos;
            }

        }

        public void Start()
        {
            if(this.CompareTag(mole1tag))
            {
                playerObj = ECSManager.FindEntityByName("Moley");
            }
            else if(this.CompareTag(mole2tag))
            {
                playerObj = ECSManager.FindEntityByName("Holey");
            }
        }


        public void Update()
        {
            SetToPlayer();
        }
    }
}
