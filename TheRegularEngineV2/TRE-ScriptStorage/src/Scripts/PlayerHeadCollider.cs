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

        private void SetToPlayer()
        {
            if (playerObj == null || ECSManager.IsValidEntity(playerObj.ID) == false)
            {
                return;
            }
            vec3 newPos = playerObj.transform.Position;
            newPos.y += playerObj.transform.Scale.y * 5f;
            transform.Position = newPos;
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
