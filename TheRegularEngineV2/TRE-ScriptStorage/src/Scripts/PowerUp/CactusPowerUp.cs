using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TRE
{
    class CactusPowerUp : Entity
    {
        private Entity playerObj;
        private string rightCactus = "RightCactus";
        private string leftCactus = "LeftCactus";

        //Scaling variables
        private float lerpSpeed = 5f;
        private vec3 defaultScale = new vec3(0f, 0f, 0f);
        private vec3 leftScale = new vec3(0f, 0f, 0f);          // ROYCE CHANGE VALUES HERE
        private vec3 rightScale = new vec3(0f, 0f, 0f);         // ROYCE CHANGE VALUES HERE
        private vec3 currScale = new vec3(0f, 0f, 0f);


        private void SetToPlayer()
        {
            if(playerObj == null || ECSManager.IsValidEntity(playerObj.ID) == false)
            {
                return;
            }
            vec3 newPos = playerObj.transform.Position;
            newPos.y += playerObj.transform.Scale.y * 5f;
            //Right Cactus Values
            if(this.CompareTag(rightCactus))
                newPos.x += playerObj.transform.Scale.z * 4f;
            //Left Cactus Values
            else if (this.CompareTag(leftCactus))
                newPos.x -= playerObj.transform.Scale.z * 4f;
            transform.Position = newPos;
        }

        private void ScaleChange()
        {
            if (this.CompareTag(rightCactus))
            {
                if(currScale.x == 0f)
                {
                    MathF.Lerp(currScale.x, rightScale.x, lerpSpeed * Time.deltaTime);
                    MathF.Lerp(currScale.y, rightScale.y, lerpSpeed * Time.deltaTime);
                    MathF.Lerp(currScale.z, rightScale.z, lerpSpeed * Time.deltaTime);
                    PhysicsSystem.ResizeBoxCollider(this.ID, currScale);
                }
                else if(currScale.x > 0f)
                {
                    MathF.Lerp(currScale.x, defaultScale.x, lerpSpeed * Time.deltaTime);
                    MathF.Lerp(currScale.y, defaultScale.y, lerpSpeed * Time.deltaTime);
                    MathF.Lerp(currScale.z, defaultScale.z, lerpSpeed * Time.deltaTime);
                    PhysicsSystem.ResizeBoxCollider(this.ID, currScale);
                }
            } 
            else if (this.CompareTag(leftCactus))
            {
                if(currScale.x == 0f)
                {
                    MathF.Lerp(currScale.x, leftScale.x, lerpSpeed * Time.deltaTime);
                    MathF.Lerp(currScale.y, leftScale.y, lerpSpeed * Time.deltaTime);
                    MathF.Lerp(currScale.z, leftScale.z, lerpSpeed * Time.deltaTime);
                    PhysicsSystem.ResizeBoxCollider(this.ID, currScale);
                }
                else if(currScale.x > 0f)
                {
                    MathF.Lerp(currScale.x, defaultScale.x, lerpSpeed * Time.deltaTime);
                    MathF.Lerp(currScale.y, defaultScale.y, lerpSpeed * Time.deltaTime);
                    MathF.Lerp(currScale.z, defaultScale.z, lerpSpeed * Time.deltaTime);
                    PhysicsSystem.ResizeBoxCollider(this.ID, currScale);
                }
            }
        }

        public void Start()
        {
             playerObj = ECSManager.FindEntityByName("Holey");
        }

        public void Update()
        {
            // set to Holey's position
            SetToPlayer();
            //Check if Holey has strawberry then allow scaling
        }
    }
}
